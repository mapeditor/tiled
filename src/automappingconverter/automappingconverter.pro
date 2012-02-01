include(../../tiled.pri)
include(../libtiled/libtiled.pri)

win32 {
    DESTDIR = ../..
} else {
    DESTDIR = ../../bin
}

macx {
    QMAKE_LIBDIR_FLAGS += -L$$OUT_PWD/../../bin/Tiled.app/Contents/Frameworks
} else:win32 {
    LIBS += -L$$OUT_PWD/../../lib
} else {
    QMAKE_LIBDIR_FLAGS += -L$$OUT_PWD/../../lib
}

# Make sure the executable can find libtiled
!win32:!macx {
    QMAKE_RPATHDIR += \$\$ORIGIN/../lib

    # It is not possible to use ORIGIN in QMAKE_RPATHDIR, so a bit manually
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$$join(QMAKE_RPATHDIR, ":")\'
    QMAKE_RPATHDIR =
}

QT       += core gui

TARGET = automappingconverter
TEMPLATE = app


SOURCES += main.cpp \
    converterdatamodel.cpp \
    convertercontrol.cpp \
    converterwindow.cpp

HEADERS  += \
    converterdatamodel.h \
    convertercontrol.h \
    converterwindow.h

FORMS    += \
    converterwindow.ui
