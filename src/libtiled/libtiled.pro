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
    # With Qt 4 it was enough to include zlib, since the symbols were available
    # in Qt. Qt 5 no longer exposes zlib symbols, so it needs to be linked.
    # Get the installer at:
    #
    #   http://gnuwin32.sourceforge.net/packages/zlib.htm
    #
    greaterThan(QT_MAJOR_VERSION, 4) {
        # If there is an environment variable, take that
        isEmpty(ZLIB_PATH):ZLIB_PATH = "$$(ZLIB_PATH)"

        # When the variable is not set, check for common install locations
        isEmpty(ZLIB_PATH):exists("C:/Program Files (x86)/GnuWin32/include/zlib.h") {
            ZLIB_PATH = "C:/Program Files (x86)/GnuWin32"
        }
        isEmpty(ZLIB_PATH):exists("C:/Program Files/GnuWin32/include/zlib.h") {
            ZLIB_PATH = "C:/Program Files/GnuWin32"
        }

        isEmpty(ZLIB_PATH) {
            error("ZLIB_PATH not defined and could not be auto-detected")
        }

        INCLUDEPATH += $${ZLIB_PATH}/include
        win32-g++*:LIBS += $${ZLIB_PATH}/lib/libz.a
        win32-msvc*:LIBS += $${ZLIB_PATH}/lib/zlib.lib
    } else {
        INCLUDEPATH += ../zlib
    }
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
    imagelayer.cpp \
    isometricrenderer.cpp \
    layer.cpp \
    map.cpp \
    mapobject.cpp \
    mapreader.cpp \
    maprenderer.cpp \
    mapwriter.cpp \
    objectgroup.cpp \
    orthogonalrenderer.cpp \
    properties.cpp \
    staggeredrenderer.cpp \
    tile.cpp \
    tilelayer.cpp \
    tileset.cpp
HEADERS += compression.h \
    consoleinterface.h \
    gidmapper.h \
    imagelayer.h \
    isometricrenderer.h \
    layer.h \
    map.h \
    mapobject.h \
    mapreader.h \
    mapreaderinterface.h \
    maprenderer.h \
    mapwriter.h \
    mapwriterinterface.h \
    object.h \
    objectgroup.h \
    orthogonalrenderer.h \
    properties.h \
    staggeredrenderer.h \
    terrain.h \
    tile.h \
    tiled.h \
    tiled_global.h \
    tilelayer.h \
    tileset.h

contains(INSTALL_HEADERS, yes) {
    headers.files = $${HEADERS}
    headers.path = $${PREFIX}/include/tiled
    INSTALLS += headers
}

macx {
    contains(QT_CONFIG, ppc):CONFIG += x86 \
        ppc
}
