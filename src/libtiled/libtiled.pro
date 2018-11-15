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
    DEFINES += _USE_MATH_DEFINES
} else {
    # On other platforms it is necessary to link to zlib explicitly
    LIBS += -lz
}

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
DEFINES += TILED_LIBRARY

contains(QT_CONFIG, reduce_exports): CONFIG += hide_symbols

include(./libtiled-src.pri)

contains(INSTALL_HEADERS, yes) {
    headers.files = $${HEADERS}
    headers.path = $${PREFIX}/include/tiled
    INSTALLS += headers
}
