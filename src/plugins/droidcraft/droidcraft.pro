include(../plugin.pri)

DEFINES += DROIDCRAFT_LIBRARY
contains(QT_VERSION, ^5\\..*) {    
OTHER_FILES = plugin.json
}

SOURCES += droidcraftplugin.cpp
HEADERS += droidcraftplugin.h\
        droidcraft_global.h

RESOURCES += \
    droidcraft.qrc
