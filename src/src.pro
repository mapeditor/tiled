include(../tiled.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = \
    libtiled \
    tiled \
    plugins \
    tmxviewer \
    tmxrasterizer \
    terraingenerator

tiled_quick {
    minQtVersion(5, 6, 0) {
        SUBDIRS += tiledquickplugin
    }
    minQtVersion(5, 10, 0) {
        SUBDIRS += tiledquick
    }
}
