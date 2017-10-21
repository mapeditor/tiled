include(../../tiled.pri)
include(../libtiled/libtiled.pri)

TEMPLATE = app
TARGET = tmxrasterizer
target.path = $${PREFIX}/bin
INSTALLS += target
CONFIG += console

win32 {
    DESTDIR = ../..
} else {
    DESTDIR = ../../bin
}

macx {
    CONFIG -= app_bundle
    QMAKE_LIBDIR += $$OUT_PWD/../../bin/Tiled.app/Contents/Frameworks
} else:win32 {
    LIBS += -L$$OUT_PWD/../../lib
} else {
    QMAKE_LIBDIR = $$OUT_PWD/../../lib $$QMAKE_LIBDIR
}

# Make sure the executable can find libtiled
!win32:!macx:!cygwin:contains(RPATH, yes) {
    QMAKE_RPATHDIR += \$\$ORIGIN/../lib

    # It is not possible to use ORIGIN in QMAKE_RPATHDIR, so a bit manually
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$$join(QMAKE_RPATHDIR, ":")\'
    QMAKE_RPATHDIR =
}

SOURCES += main.cpp \
         tmxrasterizer.cpp

HEADERS += tmxrasterizer.h

manpage.path = $${PREFIX}/share/man/man1/
manpage.files += ../../man/tmxrasterizer.1
INSTALLS += manpage
