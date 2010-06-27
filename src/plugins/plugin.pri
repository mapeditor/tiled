isEmpty(TARGET) {
    error("plugin.pri: You must provide a TARGET")
}

TEMPLATE = lib
CONFIG += plugin
contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols
INCLUDEPATH += $$PWD/../tiled
DEPENDPATH += $$PWD/../tiled
win32 {
    DESTDIR = $$OUT_PWD/../../../plugins/tiled
} else {
    DESTDIR = $$OUT_PWD/../../../lib/tiled/plugins
}

include(../../tiled.pri)
target.path = $${PREFIX}/lib/tiled/plugins
INSTALLS += target

include(../libtiled/libtiled.pri)
LIBS += -L$$OUT_PWD/../../../lib
