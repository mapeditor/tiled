TEMPLATE = lib
TARGET = tiled
win32 {
    DESTDIR = ../../bin
} else {
    DESTDIR = ../../lib
}
DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
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
    tilelayer.h \
    tileset.h
mac {
    contains(QT_CONFIG, ppc):CONFIG += x86 \
        ppc
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
}

