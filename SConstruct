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

    if env['PLATFORM'] == 'win32':
        env.Append(LINKFLAGS =
                   ['/NODEFAULTLIB:LIBCMTD', '/NODEFAULTLIB:MSVCRT'])
        env.Replace(CPPDEFINES = ['STATIC', 'QT_STATIC', 'QT_NODLL'])
        env.Append(LIBS = ('winspool gdi32 imm32 advapi32 comdlg32 ws2_32 ' +
                           'shell32 winmm').split())

    polarssl_home = os.environ.get('POLARSSL_HOME')
    if polarssl_home:
        env.Append(LIBPATH = [polarssl_home])
        env.Append(CPPPATH = [polarssl_home + '/include'])

    if not conf.CheckLib('polarssl') or not conf.CheckHeader('polarssl/aes.h'):
        raise Exception, 'Need PolarSSL'

    #env.Append(CPPDEFINES = ['DEBUG'])

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
src += env.Qrc4('build/resources.cpp', 'qt/resources.qrc')


# Build
p = env.Program(name, src)
Depends(p, uic)
Default(p)


# Clean
Clean(p, ['build', 'config.log', 'dist.txt', 'package.txt'])
