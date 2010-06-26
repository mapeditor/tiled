isEmpty(TARGET) {
    error("plugin.pri: You must provide a TARGET")
}

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += $$PWD/../tiled
DEPENDPATH += $$PWD/../tiled
DESTDIR = $$PWD/../../lib/tiled/plugins

TARGET = $$qtLibraryTarget($$TARGET)

include(../libtiled/libtiled.pri)
win32 {
    LIBS += -L$$OUT_PWD/../../../bin
} else {
    LIBS += -L$$OUT_PWD/../../../lib
}
