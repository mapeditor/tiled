include(../plugin.pri)

DEFINES += DROIDCRAFT_LIBRARY

greaterThan(QT_MAJOR_VERSION, 4) {
    OTHER_FILES = plugin.json
}

SOURCES += droidcraftplugin.cpp
HEADERS += droidcraftplugin.h\
        droidcraft_global.h

RESOURCES += \
    droidcraft.qrc
