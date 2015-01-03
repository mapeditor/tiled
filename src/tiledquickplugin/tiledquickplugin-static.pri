include($$PWD/../libtiled/libtiled-static.pri)

INCLUDEPATH += $$PWD

QT += qml quick
DEFINES += STATIC_TILEDQUICKPLUGIN

# Input
SOURCES += \
    $$PWD/tiledquickplugin.cpp \
    $$PWD/mapitem.cpp \
    $$PWD/maploader.cpp \
    $$PWD/tilelayeritem.cpp \
    $$PWD/tilesnode.cpp

HEADERS += \
    $$PWD/tiledquickplugin.h \
    $$PWD/mapitem.h \
    $$PWD/maploader.h \
    $$PWD/tilelayeritem.h \
    $$PWD/tilesnode.h
