import os, sys

name = 'myNXTWalletViewer'

# Setup
env = Environment(ENV = os.environ)
conf = env.Configure()

# Qt4 tool
env.Tool('qt4', toolpath = ['./config'])


if not env.GetOption('clean'):
    # Qt
    env.EnableQt4Modules('QtCore QtGui'.split())

    conf.CheckLib('polarssl')

    env.Append(CPPDEFINES = ['DEBUG'])

conf.Finish()

# Source
src = Glob('src/*.cpp') + Glob('src/*.c')

# Build in 'build'
import re
VariantDir('build', 'src')
src = map(lambda path: re.sub(r'^src/', 'build/', str(path)), src)
env.AppendUnique(CPPPATH = ['#/build'])


# Qt
uic = [env.Uic4('build/ui_mainwindow.h', 'qt/mainwindow.ui')]
#qrc = env.Qrc4('build/qrc_resources.cpp', 'qt/resources.qrc')
#src += qrc


# Build
p = env.Program(name, src)
Depends(p, uic)
Default(p)


# Clean
Clean(p, ['build', 'config.log', 'dist.txt', 'package.txt'])
