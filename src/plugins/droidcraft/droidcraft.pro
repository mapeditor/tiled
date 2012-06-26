include(../plugin.pri)

contains(QT_VERSION, ^5\\..*) {    
OTHER_FILES = plugin.json
} else {
DEFINES += DROIDCRAFT_LIBRARY
}
SOURCES += droidcraftplugin.cpp
HEADERS += droidcraftplugin.h\
        droidcraft_global.h

RESOURCES += \
    droidcraft.qrc
