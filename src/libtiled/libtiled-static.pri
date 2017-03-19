INCLUDEPATH += $$PWD

HEADERS += $$PWD/compression.h \
    $$PWD/gidmapper.h \
    $$PWD/imagelayer.h \
    $$PWD/isometricrenderer.h \
    $$PWD/layer.h \
    $$PWD/map.h \
    $$PWD/mapobject.h \
    $$PWD/mapreader.h \
    $$PWD/maprenderer.h \
    $$PWD/mapwriter.h \
    $$PWD/object.h \
    $$PWD/objectgroup.h \
    $$PWD/orthogonalrenderer.h \
    $$PWD/properties.h \
    $$PWD/staggeredrenderer.h \
    $$PWD/terrain.h \
    $$PWD/tile.h \
    $$PWD/tiled.h \
    $$PWD/tiled_global.h \
    $$PWD/tilelayer.h \
    $$PWD/tileset.h \
    $$PWD/logginginterface.h \
    $$PWD/hexagonalrenderer.h \
    $$PWD/mapformat.h \
    $$PWD/maptovariantconverter.h \
    $$PWD/plugin.h \
    $$PWD/pluginmanager.h \
    $$PWD/tilesetformat.h \
    $$PWD/varianttomapconverter.h \
    $$PWD/imagereference.h

SOURCES += \
    $$PWD/compression.cpp \
    $$PWD/gidmapper.cpp \
    $$PWD/hexagonalrenderer.cpp \
    $$PWD/isometricrenderer.cpp \
    $$PWD/map.cpp \
    $$PWD/mapobject.cpp \
    $$PWD/mapreader.cpp \
    $$PWD/maprenderer.cpp \
    $$PWD/maptovariantconverter.cpp \
    $$PWD/mapwriter.cpp \
    $$PWD/objectgroup.cpp \
    $$PWD/orthogonalrenderer.cpp \
    $$PWD/plugin.cpp \
    $$PWD/pluginmanager.cpp \
    $$PWD/properties.cpp \
    $$PWD/staggeredrenderer.cpp \
    $$PWD/tile.cpp \
    $$PWD/tilelayer.cpp \
    $$PWD/tileset.cpp \
    $$PWD/tilesetformat.cpp \
    $$PWD/varianttomapconverter.cpp \
    $$PWD/layer.cpp \
    $$PWD/imagelayer.cpp \
    $$PWD/imagereference.cpp

DEFINES += TILED_LIBRARY

#OBJECTS_DIR = tmp/tiled
#MOC_DIR = tmp/tiled

