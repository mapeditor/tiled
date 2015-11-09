include(../../tiled.pri)

TEMPLATE = lib
TARGET = tiled
target.path = $${LIBDIR}
INSTALLS += target
macx {
    DESTDIR = ../../bin/Tiled.app/Contents/Frameworks
    QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
} else {
    DESTDIR = ../../lib
}
DLLDESTDIR = ../..

win32 {
    QMAKE_PROJECT_NAME = libtiled
} else {
    # On other platforms it is necessary to link to zlib explicitly
    LIBS += -lz
}

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
DEFINES += TILED_LIBRARY
contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols

SOURCES += compression.cpp \
    gidmapper.cpp \
    hexagonalrenderer.cpp \
    imagelayer.cpp \
    imagereference.cpp \
    isometricrenderer.cpp \
    layer.cpp \
    map.cpp \
    mapobject.cpp \
    mapreader.cpp \
    maprenderer.cpp \
    maptovariantconverter.cpp \
    mapwriter.cpp \
    objectgroup.cpp \
    orthogonalrenderer.cpp \
    plugin.cpp \
    pluginmanager.cpp \
    properties.cpp \
    staggeredrenderer.cpp \
    tile.cpp \
    tilelayer.cpp \
    tileset.cpp \
    tilesetformat.cpp \
    varianttomapconverter.cpp
HEADERS += compression.h \
    gidmapper.h \
    hexagonalrenderer.h \
    imagelayer.h \
    imagereference.h \
    isometricrenderer.h \
    layer.h \
    logginginterface.h \
    map.h \
    mapformat.h \
    mapobject.h \
    mapreader.h \
    maprenderer.h \
    maptovariantconverter.h \
    mapwriter.h \
    object.h \
    objectgroup.h \
    orthogonalrenderer.h \
    plugin.h \
    pluginmanager.h \
    properties.h \
    staggeredrenderer.h \
    terrain.h \
    tile.h \
    tiled.h \
    tiled_global.h \
    tilelayer.h \
    tileset.h \
    tilesetformat.h \
    varianttomapconverter.h

contains(INSTALL_HEADERS, yes) {
    headers.files = $${HEADERS}
    headers.path = $${PREFIX}/include/tiled
    INSTALLS += headers
}
