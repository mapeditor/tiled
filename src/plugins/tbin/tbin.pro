include(../plugin.pri)

DEFINES += JSON_LIBRARY

SOURCES += tbinplugin.cpp \
    tbin/Map.cpp

HEADERS += tbin_global.h \
    tbinplugin.h \
    tbin/Layer.hpp \
    tbin/Map.hpp \
    tbin/PropertyValue.hpp \
    tbin/Tile.hpp \
    tbin/TileSheet.hpp
