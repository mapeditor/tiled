TEMPLATE = app
TARGET = tiled
DESTDIR = ../bin
DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
QT += core \
    gui \
    xml
CONFIG += debug
SOURCES += aboutdialog.cpp \
    brushitem.cpp \
    compression.cpp \
    layer.cpp \
    layerdock.cpp \
    layermodel.cpp \
    main.cpp \
    mainwindow.cpp \
    map.cpp \
    mapdocument.cpp \
    mapobject.cpp \
    mapobjectitem.cpp \
    mapscene.cpp \
    mapview.cpp \
    objectgroup.cpp \
    painttilelayer.cpp \
    propertiesdialog.cpp \
    propertiesmodel.cpp \
    resizehelper.cpp \
    resizedialog.cpp \
    tileselectionitem.cpp \
    tileselectionmodel.cpp \
    tileset.cpp \
    tilesetdock.cpp \
    tilesetmanager.cpp \
    tilesetmodel.cpp \
    tilesetview.cpp \
    tilelayer.cpp \
    tilelayeritem.cpp \
    tsxtilesetreader.cpp \
    xmlmapreader.cpp \
    xmlmapwriter.cpp \
    changeproperties.cpp \
    movelayer.cpp \
    tilepainter.cpp \
    newmapdialog.cpp \
    newtilesetdialog.cpp \
    objectgroupitem.cpp \
    movemapobject.cpp \
    resizemapobject.cpp \
    addremovemapobject.cpp \
    addremovelayer.cpp \
    propertiesview.cpp \
    renamelayer.cpp \
    resizelayer.cpp \
    resizemap.cpp \
    objectpropertiesdialog.cpp \
    changemapobject.cpp \
    maprenderer.cpp
HEADERS += aboutdialog.h \
    brushitem.h \
    compression.h \
    layer.h \
    layerdock.h \
    layermodel.h \
    mainwindow.h \
    mapreaderinterface.h \
    mapwriterinterface.h \
    map.h \
    mapdocument.h \
    mapobject.h \
    mapobjectitem.h \
    mapscene.h \
    mapview.h \
    objectgroup.h \
    painttilelayer.h \
    propertiesdialog.h \
    propertiesmodel.h \
    resizedialog.h \
    resizehelper.h \
    tile.h \
    tileselectionitem.h \
    tileselectionmodel.h \
    tileset.h \
    tilesetdock.h \
    tilesetmanager.h \
    tilesetmodel.h \
    tilesetview.h \
    tilelayer.h \
    tilelayeritem.h \
    tsxtilesetreader.h \
    xmlmapreader.h \
    xmlmapwriter.h \
    changeproperties.h \
    movelayer.h \
    tilepainter.h \
    newmapdialog.h \
    newtilesetdialog.h \
    objectgroupitem.h \
    movemapobject.h \
    resizemapobject.h \
    addremovemapobject.h \
    addremovelayer.h \
    propertiesview.h \
    renamelayer.h \
    resizelayer.h \
    resizemap.h \
    objectpropertiesdialog.h \
    changemapobject.h \
    maprenderer.h
FORMS += aboutdialog.ui \
    mainwindow.ui \
    resizedialog.ui \
    propertiesdialog.ui \
    newmapdialog.ui \
    newtilesetdialog.ui
RESOURCES += tiled.qrc
TRANSLATIONS = translations/tiled_nl.ts

mac {
    TARGET = Tiled
    LIBS += -lz
}

