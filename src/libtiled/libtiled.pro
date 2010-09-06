include(../../tiled.pri)

TEMPLATE = lib
TARGET = tiled
target.path = $${PREFIX}/lib
INSTALLS += target
macx {
    DESTDIR = ../../bin/Tiled.app/Contents/Frameworks
    QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
    LIBS += -lz
} else {
    DESTDIR = ../../lib
}
DLLDESTDIR = ../..

win32:INCLUDEPATH += $$(QTDIR)/src/3rdparty/zlib

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
DEFINES += TILED_LIBRARY
contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols
OBJECTS_DIR = .obj
SOURCES += compression.cpp \
    layer.cpp \
    map.cpp \
    mapobject.cpp \
    objectgroup.cpp \
    tilelayer.cpp \
    tileset.cpp \
    mapreader.cpp \
    properties.cpp
HEADERS += compression.h \
    layer.h \
    map.h \
    mapobject.h \
    objectgroup.h \
    tile.h \
    tiled_global.h \
    tilelayer.h \
    tileset.h \
    mapreader.h \
    properties.h \
    object.h
mac {
    contains(QT_CONFIG, ppc):CONFIG += x86 \
        ppc
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
}
