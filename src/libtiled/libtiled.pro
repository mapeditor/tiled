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

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
DEFINES += TILED_LIBRARY
contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols
OBJECTS_DIR = .obj
SOURCES += layer.cpp \
    map.cpp \
    mapobject.cpp \
    objectgroup.cpp \
    tilelayer.cpp \
    tileset.cpp
HEADERS += layer.h \
    map.h \
    mapobject.h \
    objectgroup.h \
    tile.h \
    tiled_global.h \
    tilelayer.h \
    tileset.h
mac {
    contains(QT_CONFIG, ppc):CONFIG += x86 \
        ppc
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
}
