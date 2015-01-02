include(../tiled.pri)

TEMPLATE  = subdirs
CONFIG   += ordered

SUBDIRS = \
    libtiled \
    tiled \
    plugins \
    tmxviewer \
    tmxrasterizer \
    automappingconverter

minQtVersion(5, 4, 0) {
    SUBDIRS += \
        tiledquickplugin \
        tiledquick
}
