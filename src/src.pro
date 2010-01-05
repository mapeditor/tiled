TEMPLATE = app
TARGET = tiled
DESTDIR = ../bin
DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII
INCLUDEPATH += .
QT += core \
    gui
CONFIG += debug
MOC_DIR = .moc
UI_DIR = .uic
RCC_DIR = .rcc
OBJECTS_DIR = .obj
SOURCES += aboutdialog.cpp \
    brushitem.cpp \
    compression.cpp \
    languagemanager.cpp \
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
    preferencesdialog.cpp \
    preferences.cpp \
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
    tmxmapreader.cpp \
    tmxmapwriter.cpp \
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
    stampbrush.cpp \
    toolmanager.cpp \
    eraser.cpp \
    erasetiles.cpp \
    saveasimagedialog.cpp \
    utils.cpp \
    colorbutton.cpp \
    undodock.cpp \
    selectiontool.cpp \
    abstracttiletool.cpp \
    abstracttool.cpp \
    changeselection.cpp \
    clipboardmanager.cpp \
    orthogonalrenderer.cpp \
    isometricrenderer.cpp \
    offsetlayer.cpp \
    offsetmapdialog.cpp \
    bucketfilltool.cpp \
    filltiles.cpp \
    objectgrouppropertiesdialog.cpp \
    changeobjectgroupproperties.cpp
HEADERS += aboutdialog.h \
    brushitem.h \
    compression.h \
    languagemanager.h \
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
    preferencesdialog.h \
    preferences.h \
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
    tmxmapreader.h \
    tmxmapwriter.h \
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
    maprenderer.h \
    abstracttool.h \
    stampbrush.h \
    toolmanager.h \
    eraser.h \
    erasetiles.h \
    saveasimagedialog.h \
    utils.h \
    colorbutton.h \
    undodock.h \
    selectiontool.h \
    abstracttiletool.h \
    changeselection.h \
    clipboardmanager.h \
    undocommands.h \
    orthogonalrenderer.h \
    isometricrenderer.h \
    offsetlayer.h \
    offsetmapdialog.h \
    bucketfilltool.h \
    filltiles.h \
    objectgrouppropertiesdialog.h \
    changeobjectgroupproperties.h
FORMS += aboutdialog.ui \
    mainwindow.ui \
    resizedialog.ui \
    preferencesdialog.ui \
    propertiesdialog.ui \
    newmapdialog.ui \
    newtilesetdialog.ui \
    saveasimagedialog.ui \
    offsetmapdialog.ui
RESOURCES += tiled.qrc
mac {
    TARGET = Tiled
    LIBS += -lz
}
win32:INCLUDEPATH += $$(QTDIR)/src/3rdparty/zlib
