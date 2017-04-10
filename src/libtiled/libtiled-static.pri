include(./libtiled-src.pri)

!win32 {
    # On other platforms it is necessary to link to zlib explicitly
    LIBS += -lz
}

DEFINES += TILED_LIBRARY
