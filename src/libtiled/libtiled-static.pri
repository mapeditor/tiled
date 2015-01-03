INCLUDEPATH += $$PWD

win32 {
    lessThan(QT_MAJOR_VERSION, 5) {
        INCLUDEPATH += $$PWD/../zlib
    }
} else {
    # On other platforms it is necessary to link to zlib explicitly
    LIBS += -lz
}

SOURCES += $$PWD/compression.cpp \
    $$PWD/gidmapper.cpp \
    $$PWD/imagelayer.cpp \
    $$PWD/isometricrenderer.cpp \
    $$PWD/layer.cpp \
    $$PWD/map.cpp \
    $$PWD/mapobject.cpp \
    $$PWD/mapreader.cpp \
    $$PWD/maprenderer.cpp \
    $$PWD/mapwriter.cpp \
    $$PWD/objectgroup.cpp \
    $$PWD/orthogonalrenderer.cpp \
    $$PWD/properties.cpp \
    $$PWD/staggeredrenderer.cpp \
    $$PWD/tile.cpp \
    $$PWD/tilelayer.cpp \
    $$PWD/tileset.cpp \
    $$PWD/hexagonalrenderer.cpp
HEADERS += $$PWD/compression.h \
    $$PWD/gidmapper.h \
    $$PWD/imagelayer.h \
    $$PWD/isometricrenderer.h \
    $$PWD/layer.h \
    $$PWD/map.h \
    $$PWD/mapobject.h \
    $$PWD/mapreader.h \
    $$PWD/mapreaderinterface.h \
    $$PWD/maprenderer.h \
    $$PWD/mapwriter.h \
    $$PWD/mapwriterinterface.h \
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
    $$PWD/hexagonalrenderer.h
