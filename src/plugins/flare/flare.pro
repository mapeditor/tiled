include(../plugin.pri)

DEFINES += FLARE_LIBRARY

contains(QT_VERSION, ^5\\..*) {    
OTHER_FILES = plugin.json
} else {
DEFINES -= FLARE_LIBRARY
}

HEADERS += \
    flare_global.h \
    flareplugin.h

SOURCES += \
    flareplugin.cpp


