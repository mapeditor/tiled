include(../../tiled.pri)
include(../libtiled/libtiled.pri)

win32 {
    DESTDIR = ../..
} else {
    DESTDIR = ../../bin
}

macx {
    QMAKE_LIBDIR += $$OUT_PWD/../../bin/Tiled.app/Contents/Frameworks
} else:win32 {
    LIBS += -L$$OUT_PWD/../../lib
} else {
    QMAKE_LIBDIR = $$OUT_PWD/../../lib $$QMAKE_LIBDIR
}

# Make sure the executable can find libtiled
!win32:!macx:contains(RPATH, yes) {
    QMAKE_RPATHDIR += \$\$ORIGIN/../lib

    # It is not possible to use ORIGIN in QMAKE_RPATHDIR, so a bit manually
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$$join(QMAKE_RPATHDIR, ":")\'
    QMAKE_RPATHDIR =
}

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

TARGET = automappingconverter
TEMPLATE = app

target.path = $${PREFIX}/bin
INSTALLS += target


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

manpage.path = $${PREFIX}/share/man/man1/
manpage.files += ../../docs/automappingconverter.1
INSTALLS += manpage

