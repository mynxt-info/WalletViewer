#include "WalletDecrypter.h"

// This causes Windows to not automatically create a console
#if defined(_WIN32) && !defined(DEBUG)
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

#include <QApplication>

#include <exception>
#include <iostream>

using namespace std;


int main(int argc, char *argv[]) {
  try {

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("myNXT.info");
    QCoreApplication::setApplicationName("Wallet Decrypter");

    myNXT::WalletDecrypter win;

    win.show();
    app.exec();

    return 0;

  } catch (exception &e) {
    cerr << "Exception: " << e.what() << endl;
  }

  return -1;
}
