#ifndef MYNXT_WALLET_DECRYPTER_H
#define MYNXT_WALLET_DECRYPTER_H

#include "ui_mainwindow.h"
#include "StdTypes.h"

#include <QtGui>

#include <string>

#define PKCS5_ITERATIONS 1499


namespace myNXT {
  class WalletDecrypter : public QMainWindow {
    Q_OBJECT;

    Ui::MainWindow ui;
    QFileDialog fileDialog;

    typedef enum {
      JSON_STATE_START,
      JSON_STATE_DICT,
      JSON_STATE_ACCOUNTS_KEY,
      JSON_STATE_ACCOUNTS_ARRAY,
      JSON_STATE_ACCOUNT_DICT,
      JSON_STATE_PASSWORD_KEY,
      JSON_STATE_ID_KEY,
    } json_state_t;

    std::string parseError;
    json_state_t state;
    std::string password;
    std::string id;

    QTableWidgetItem *selectedItem;

  public:
    WalletDecrypter();

    std::string getPassphrase();
    void computeKey(const std::string &passphrase, const uint8_t salt[24],
                    uint8_t key[32]);
    std::string decrypt(const std::string &path, const std::string &passphrase);
    void addAccount(const std::string &id, const std::string &password);

  protected slots:
    void on_openWalletPushButton_clicked();
    void on_accountsTableWidget_customContextMenuRequested(QPoint point);
    void on_actionCopy_activated();

  public:
    int jsonCallback(int type, const char *data, uint32_t length);
  };
}

#endif // MYNXT_WALLET_DECRYPTER_H

