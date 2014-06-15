myNXTWalletViewer
=================

Wallet decrypter and viewer for http://wallet.myNXT.info/

Dependencies
============
To build this software you will need:

  * SCons
  * Qt 4
  * PolarSSL

Build in Debian or Ubuntu Linux
===============================

    sudo apt-get install scons libqt4-dev libpolarssl-dev git
    git clone https://github.com/mynxt-info/myNXTWalletViewer.git
    cd myNXTWalletViewer
    scons

Build in Windows
================

  1. Download and install scons using the Windows installer:
    http://www.scons.org/download.php

  2. Download and install Qt4 using the Windows installer:
    http://qt-project.org/downloads

  3. Download the PolarSSL source code.
    https://polarssl.org/

  4. Build the PolarSSL solution in Visual Studio.
    https://polarssl.org/kb/compiling-and-building/how-do-i-build-compile-polarssl

  5. Download and install Git
    http://git-scm.com/download/win

  6. Setup your environment

        set QT4DIR=C:\path\to\qt4
        set PATH=$QT4DIR\bin;%PATH%
        set POLARSSL_HOME=C:\path\to\polarssl

  7. Get the source code and build

        git clone https://github.com/mynxt-info/myNXTWalletViewer.git
        cd myNXTWalletViewer
        scons

Example
=======

Try opening the example wallet file *wallet.json.aes* with password *example*.
