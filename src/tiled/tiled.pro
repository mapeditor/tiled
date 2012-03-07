include(../../tiled.pri)
include(../libtiled/libtiled.pri)

TEMPLATE = app
TARGET = tiled
target.path = $${PREFIX}/bin
INSTALLS += target
win32 {
    DESTDIR = ../..
} else {
    DESTDIR = ../../bin
}

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}
contains(QT_CONFIG, opengl): QT += opengl

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII

macx {
    QMAKE_LIBDIR += $$OUT_PWD/../../bin/Tiled.app/Contents/Frameworks
    LIBS += -framework Foundation
} else:win32 {
    LIBS += -L$$OUT_PWD/../../lib
} else {
    QMAKE_LIBDIR += $$OUT_PWD/../../lib
}

# Make sure the Tiled executable can find libtiled
!win32:!macx:contains(RPATH, yes) {
    QMAKE_RPATHDIR += \$\$ORIGIN/../lib

    # It is not possible to use ORIGIN in QMAKE_RPATHDIR, so a bit manually
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$$join(QMAKE_RPATHDIR, ":")\'
    QMAKE_RPATHDIR =
}

SOURCES += aboutdialog.cpp \
    abstractobjecttool.cpp \
    abstracttiletool.cpp \
    abstracttool.cpp \
    addremovelayer.cpp \
    addremovemapobject.cpp \
    addremoveterrain.cpp \
    addremovetileset.cpp \
    automapper.cpp \
    automapperwrapper.cpp \
    automappingmanager.cpp \
    automappingutils.cpp  \
    brushitem.cpp \
    bucketfilltool.cpp \
    changemapobject.cpp \
    changemapproperties.cpp \
    changeimagelayerproperties.cpp \
    changeobjectgroupproperties.cpp \
    changepolygon.cpp \
    changeproperties.cpp \
    changetileselection.cpp \
    changetileterrain.cpp \
    clipboardmanager.cpp \
    colorbutton.cpp \
    commandbutton.cpp \
    command.cpp \
    commanddatamodel.cpp \
    commanddialog.cpp \
    commandlineparser.cpp \
    createobjecttool.cpp \
    documentmanager.cpp \
    editpolygontool.cpp \
    editterraindialog.cpp \
    eraser.cpp \
    erasetiles.cpp \
    filesystemwatcher.cpp \
    filltiles.cpp \
    geometry.cpp \
    imagelayeritem.cpp \
    imagelayerpropertiesdialog.cpp \
    languagemanager.cpp \
    layerdock.cpp \
    layermodel.cpp \
    main.cpp \
    mainwindow.cpp \
    mapdocumentactionhandler.cpp \
    mapdocument.cpp \
    mapobjectitem.cpp \
    mapobjectmodel.cpp \
    mappropertiesdialog.cpp \
    mapscene.cpp \
    mapsdock.cpp \
    mapview.cpp \
    minimap.cpp \
    minimapdock.cpp \
    movelayer.cpp \
    movemapobject.cpp \
    movemapobjecttogroup.cpp \
    movetileset.cpp \
    newmapdialog.cpp \
    newtilesetdialog.cpp \
    objectgroupitem.cpp \
    objectgrouppropertiesdialog.cpp \
    objectpropertiesdialog.cpp \
    objectsdock.cpp \
    objectselectiontool.cpp \
    objecttypes.cpp \
    objecttypesmodel.cpp \
    offsetlayer.cpp \
    offsetmapdialog.cpp \
    painttilelayer.cpp \
    pluginmanager.cpp \
    preferences.cpp \
    preferencesdialog.cpp \
    propertiesdialog.cpp \
    propertiesmodel.cpp \
    propertiesview.cpp \
    quickstampmanager.cpp \
    renamelayer.cpp \
    resizedialog.cpp \
    resizehelper.cpp \
    resizelayer.cpp \
    resizemap.cpp \
    resizemapobject.cpp \
    rotatemapobject.cpp \
    saveasimagedialog.cpp \
    selectionrectangle.cpp \
    stampbrush.cpp \
    terrainbrush.cpp \
    terraindock.cpp \
    terrainmodel.cpp \
    terrainview.cpp \
    tiledapplication.cpp \
    tilelayeritem.cpp \
    tilepainter.cpp \
    tileselectionitem.cpp \
    tileselectiontool.cpp \
    tilesetdock.cpp \
    tilesetmanager.cpp \
    tilesetmodel.cpp \
    tilesetview.cpp \
    tmxmapreader.cpp \
    tmxmapwriter.cpp \
    toolmanager.cpp \
    undodock.cpp \
    utils.cpp \
    zoomable.cpp

HEADERS += aboutdialog.h \
    abstractobjecttool.h \
    abstracttiletool.h \
    abstracttool.h \
    addremovelayer.h \
    addremovemapobject.h \
    addremoveterrain.h \
    addremovetileset.h \
    automapper.h \
    automapperwrapper.h \
    automappingmanager.h \
    automappingutils.h \
    brushitem.h \
    bucketfilltool.h \
    changemapobject.h \
    changemapproperties.h \
    changeimagelayerproperties.h\
    changeobjectgroupproperties.h \
    changepolygon.h \
    changeproperties.h \
    changetileselection.h \
    changetileterrain.h \
    clipboardmanager.h \
    colorbutton.h \
    commandbutton.h \
    commanddatamodel.h \
    commanddialog.h \
    command.h \
    commandlineparser.h \
    createobjecttool.h \
    documentmanager.h \
    editpolygontool.h \
    editterraindialog.h \
    eraser.h \
    erasetiles.h \
    filesystemwatcher.h \
    filltiles.h \
    geometry.h \
    imagelayeritem.h \
    imagelayerpropertiesdialog.h \
    languagemanager.h \
    layerdock.h \
    layermodel.h \
    macsupport.h \
    mainwindow.h \
    mapdocumentactionhandler.h \
    mapdocument.h \
    mapobjectitem.h \
    mapobjectmodel.h \
    mappropertiesdialog.h \
    mapscene.h \
    mapsdock.h \
    mapview.h \
    minimap.h \
    minimapdock.h \
    movelayer.h \
    movemapobject.h \
    movemapobjecttogroup.h \
    movetileset.h \
    newmapdialog.h \
    newtilesetdialog.h \
    objectgroupitem.h \
    objectgrouppropertiesdialog.h \
    objectpropertiesdialog.h \
    objectsdock.h \
    objectselectiontool.h \
    objecttypes.h \
    objecttypesmodel.h \
    offsetlayer.h \
    offsetmapdialog.h \
    painttilelayer.h \
    pluginmanager.h \
    preferencesdialog.h \
    preferences.h \
    propertiesdialog.h \
    propertiesmodel.h \
    propertiesview.h \
    quickstampmanager.h \
    rangeset.h \
    renamelayer.h \
    resizedialog.h \
    resizehelper.h \
    resizelayer.h \
    resizemap.h \
    resizemapobject.h \
    rotatemapobject.h \
    saveasimagedialog.h \
    selectionrectangle.h \
    stampbrush.h \
    terrainbrush.h \
    terraindock.h \
    terrainmodel.h \
    terrainview.h \
    tiledapplication.h \
    tilelayeritem.h \
    tilepainter.h \
    tileselectionitem.h \
    tileselectiontool.h \
    tilesetdock.h \
    tilesetmanager.h \
    tilesetmodel.h \
    tilesetview.h \
    tmxmapreader.h \
    tmxmapwriter.h \
    toolmanager.h \
    undocommands.h \
    undodock.h \
    utils.h \
    zoomable.h

macx {
    OBJECTIVE_SOURCES += macsupport.mm
}

FORMS += aboutdialog.ui \
    commanddialog.ui \
    mainwindow.ui \
    newmapdialog.ui \
    newtilesetdialog.ui \
    objectpropertiesdialog.ui \
    offsetmapdialog.ui \
    preferencesdialog.ui \
    propertiesdialog.ui \
    resizedialog.ui \
    saveasimagedialog.ui\
    editterraindialog.ui

images.path = $${PREFIX}/share/tiled/images
images.files += images/tiled-icon-32.png
INSTALLS += images

desktopfile.path = $${PREFIX}/share/applications/
desktopfile.files += ../../docs/tiled.desktop
INSTALLS += desktopfile

manpage.path = $${PREFIX}/share/man/man1/
manpage.files += ../../docs/tiled.1
INSTALLS += manpage

RESOURCES += tiled.qrc
macx {
    TARGET = Tiled
    QMAKE_INFO_PLIST = Info.plist
    ICON = images/tiled-icon-mac.icns
}
win32 {
    RC_FILE = tiled.rc
}
win32:INCLUDEPATH += .
contains(CONFIG, static) {
    DEFINES += STATIC_BUILD
    QTPLUGIN += qgif \
        qjpeg \
        qtiff
}
