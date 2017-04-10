include(../tiled.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = \
    libtiled \
    plugins \
    tmxviewer \
    tmxrasterizer \
    automappingconverter \
    terraingenerator

minQtVersion(5, 4, 0) {
    SUBDIRS += \
        tiledquickplugin \
        tiledquick
}
