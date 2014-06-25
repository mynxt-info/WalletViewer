#include "WalletDecrypter.h"

#include "json.h"

#include <polarssl/aes.h>
#include <polarssl/sha256.h>
#include <polarssl/pkcs5.h>
#include <polarssl/base64.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iomanip>

#include <sys/stat.h>

using namespace myNXT;
using namespace std;


namespace {
  template <typename T>
  struct SmartArrayPointer {
    T *ptr;
    SmartArrayPointer(T *ptr) : ptr(ptr) {}
    ~SmartArrayPointer() {delete [] ptr;}
  };

}


extern "C"
int json_callback(void *obj, int type, const char *data, uint32_t length) {
  return ((WalletDecrypter *)obj)->jsonCallback(type, data, length);
}


WalletDecrypter::WalletDecrypter() : QMainWindow(0), selectedItem(0) {
  ui.setupUi(this);
}


string WalletDecrypter::getPassphrase() {
  bool ok = false;

  QString text = QInputDialog::getText(this, "Wallet Passphrase",
                                       tr("Enter passphrase:"),
                                       QLineEdit::Password,
                                       QString(), &ok);
  return (ok && !text.isEmpty()) ? text.toAscii().data() : "";
}


void WalletDecrypter::computeKey(const string &passphrase,
                                 const uint8_t salt[24], uint8_t key[32]) {
  // Init context
  const md_info_t *info = md_info_from_type(POLARSSL_MD_SHA256);
  if (!info) throw runtime_error("Failed to get digest context info");

  md_context_t ctx;
  if (md_init_ctx(&ctx, info))
    throw runtime_error("Failed to init digest context");

  if (pkcs5_pbkdf2_hmac(&ctx, (uint8_t *)passphrase.c_str(),
                        passphrase.length(), salt, 24, PKCS5_ITERATIONS, 32,
                        key))
    throw runtime_error("Failed to compute key from passphrase");

#if DEBUG
  cout << "Salt: ";
  for (unsigned i = 0; i < 24; i++)
    cout << setfill('0') << hex << setw(2) << (unsigned)salt[i];
  cout << endl;

  cout << "Key: ";
  for (unsigned i = 0; i < 32; i++)
    cout << setfill('0') << hex << setw(2) << (unsigned)key[i];
  cout << endl;
#endif
}


string WalletDecrypter::decrypt(const string &path, const string &passphrase) {
  // Read the file
  ifstream in(path.c_str(), ios::binary | ios::ate);
  if (!in.is_open()) throw runtime_error("Failed to open " + path);
  streampos size = in.tellg();
  in.seekg(0);

  // Read salt
  uint8_t salt[24];
  in.read((char *)salt, 24);
  size -= 24;

  SmartArrayPointer<uint8_t> buf = new uint8_t[size];
  in.read((char *)buf.ptr, size);
  if (in.gcount() != size) throw runtime_error("Failed to read " + path);
  in.close();

  // Translate base 64
  for (unsigned i = 0; i < size; i++)
    switch (buf.ptr[i]) {
    case '-': buf.ptr[i] = '+'; break;
    case '_': buf.ptr[i] = '/'; break;
    case ',': buf.ptr[i] = '='; break;
    }

  // Compute Base 64 decoded size
  size_t dataSize = 0;
  if (base64_decode(0, &dataSize, buf.ptr, size) !=
      POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL)
    throw runtime_error("Failed to decode base64 size: " + path);

  // Base 64 decode
  SmartArrayPointer<uint8_t> data = new uint8_t[dataSize];
  if (base64_decode(data.ptr, &dataSize, buf.ptr, size))
    throw runtime_error("Failed to decode base64: " + path);

  // Check that data fits block size
  if ((dataSize & 15) != 0)
    throw runtime_error("Failed data not in 16-byte blocks: " + path);

  // Setup the key
  uint8_t key[32];
  computeKey(passphrase, salt, key);

  // Decrypt
  SmartArrayPointer<uint8_t> output = new uint8_t[dataSize];
  unsigned blocks = dataSize / 16;
  aes_context ctx;
  if (aes_setkey_dec(&ctx, key, 256))
    throw runtime_error("Failed to setup AES decryption");

  for (unsigned i = 0; i < blocks; i++)
    if (aes_crypt_ecb(&ctx, AES_DECRYPT, data.ptr + i * 16,
                      output.ptr + i * 16))
      throw runtime_error("Decrypt failed");

  // Subtract salt length and remove padding
  while (output.ptr[dataSize - 1] == 0) dataSize--;

  return string((char *)output.ptr, dataSize);
}


void WalletDecrypter::addAccount(const string &id, const string &password) {
  int count = ui.accountsTableWidget->rowCount();
  ui.accountsTableWidget->setRowCount(count + 1);

  QTableWidgetItem *idItem = new QTableWidgetItem(id.c_str());
  QTableWidgetItem *passwordItem = new QTableWidgetItem(password.c_str());

  ui.accountsTableWidget->setItem(count, 0, idItem);
  ui.accountsTableWidget->setItem(count, 1, passwordItem);
}


void WalletDecrypter::on_openWalletPushButton_clicked() {
  try {
    fileDialog.setWindowTitle("Open Encrypted Wallet File");
    fileDialog.setNameFilter("Encrypted Walle File (*.json.aes);;"
                             "All Files (*.*)");
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

    if (fileDialog.exec() != QFileDialog::Accepted) return;

    QStringList names = fileDialog.selectedFiles();
    if (!names.size()) return;
    string path = names.at(0).toAscii().data();

    struct stat statBuf;
    if (stat(path.c_str(), &statBuf) == -1)
      throw runtime_error("File '" + path + "' does not exist");

#if DEBUG
    cout << "Opening: " << path << endl;
#endif

    string passphrase = getPassphrase();
    if (passphrase.empty()) return;

    string data = decrypt(path, passphrase);

#ifdef DEBUG
    cout << "'" << data << "'" << endl;
#endif

    // Clear accounds table
    ui.accountsTableWidget->setRowCount(0);

    // Decode JSON
    state = JSON_STATE_START;
    parseError.clear();

    json_parser parser;
    if (json_parser_init(&parser, 0, json_callback, this))
      throw runtime_error("Failed to init JSON parser");

    int ret;
    if ((ret = json_parser_string(&parser, data.c_str(), data.length(), 0))) {
#ifdef DEBUG
      cerr << "Failed to parse JSON:" << ret << ": " << parseError << endl;
#endif
      throw runtime_error("Invalid passphrase or wallet file.");
    }

    json_parser_free(&parser);

    // Update UI
    ui.walletLabel->setText(path.c_str());
    ui.jsonPlainTextEdit->setPlainText(data.c_str());

  } catch (const exception &e) {
    cerr << "ERROR: " << e.what() << endl;
    QMessageBox::critical(this, "Error", e.what());
  }
}


void WalletDecrypter::
on_accountsTableWidget_customContextMenuRequested(QPoint point) {
  QList<QTableWidgetItem *> items = ui.accountsTableWidget->selectedItems();
  if (!items.size()) return;
  selectedItem = items[0];

  QMenu menu(this);
  menu.addAction(ui.actionCopy);
  menu.exec(ui.accountsTableWidget->viewport()->mapToGlobal(point));
}


void WalletDecrypter::on_actionCopy_activated() {
  if (!selectedItem) return;
  QApplication::clipboard()->setText(selectedItem->text());
}


int WalletDecrypter::jsonCallback(int type, const char *data, uint32_t length) {
#ifdef DEBUG
  cout << "type=" << type << " state=" << state << " '"
       << string(data, length) << "'" << endl;
#endif

  try {
    switch (state) {
    case JSON_STATE_START:
      if (type != JSON_OBJECT_BEGIN) throw runtime_error("Expected dict");
      state = JSON_STATE_DICT;
      break;

    case JSON_STATE_DICT:
      if (type == JSON_KEY && string(data, length) == "accounts")
        state = JSON_STATE_ACCOUNTS_KEY;
      else if (type == JSON_OBJECT_END) state = JSON_STATE_START;
      break;

    case JSON_STATE_ACCOUNTS_KEY:
      if (type != JSON_ARRAY_BEGIN)
        throw runtime_error("Expected accounts array");
      state = JSON_STATE_ACCOUNTS_ARRAY;
      break;

    case JSON_STATE_ACCOUNTS_ARRAY:
      if (type == JSON_OBJECT_BEGIN) {
        password.clear();
        id.clear();
        state = JSON_STATE_ACCOUNT_DICT;

      } else if (type == JSON_ARRAY_END) state = JSON_STATE_DICT;
      break;

    case JSON_STATE_ACCOUNT_DICT:
      if (type == JSON_KEY) {
        string key(data, length);
        if (key == "password") state = JSON_STATE_PASSWORD_KEY;
        else if (key == "id") state = JSON_STATE_ID_KEY;

      } else if (type == JSON_OBJECT_END) {
        addAccount(id, password);
        state = JSON_STATE_ACCOUNTS_ARRAY;
      }
      break;

    case JSON_STATE_PASSWORD_KEY:
      if (type != JSON_STRING) throw runtime_error("Expected password string");
      password = string(data, length);
      state = JSON_STATE_ACCOUNT_DICT;
      break;

    case JSON_STATE_ID_KEY:
      if (type != JSON_STRING) throw runtime_error("Expected id string");
      id = string(data, length);
      state = JSON_STATE_ACCOUNT_DICT;
      break;
    }
    return 0;

  } catch (const exception &e) {parseError = e.what();}

  return 1;
}
