include(../plugin.pri)

DEFINES += FLARE_LIBRARY

greaterThan(QT_MAJOR_VERSION, 4) {
    OTHER_FILES = plugin.json
}

HEADERS += \
    flare_global.h \
    flareplugin.h

SOURCES += \
    flareplugin.cpp


