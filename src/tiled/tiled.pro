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
contains(QT_CONFIG, opengl): QT += opengl

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII

macx {
    QMAKE_LIBDIR_FLAGS += -L$$OUT_PWD/../../bin/Tiled.app/Contents/Frameworks
    LIBS += -framework Foundation
} else:win32 {
    LIBS += -L$$OUT_PWD/../../lib
} else {
    QMAKE_LIBDIR_FLAGS += -L$$OUT_PWD/../../lib
}

# Make sure the Tiled executable can find libtiled
!win32:!macx {
    QMAKE_RPATHDIR += \$\$ORIGIN/../lib

    # It is not possible to use ORIGIN in QMAKE_RPATHDIR, so a bit manually
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$$join(QMAKE_RPATHDIR, ":")\'
    QMAKE_RPATHDIR =
}

MOC_DIR = .moc
UI_DIR = .uic
RCC_DIR = .rcc
OBJECTS_DIR = .obj

SOURCES += aboutdialog.cpp \
    abstractobjecttool.cpp \
    abstracttiletool.cpp \
    abstracttool.cpp \
    addremovelayer.cpp \
    addremovemapobject.cpp \
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
    mapview.cpp \
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
    saveasimagedialog.cpp \
    selectionrectangle.cpp \
    stampbrush.cpp \
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
    zoomable.cpp \
    terrainbrush.cpp \
    terraindock.cpp \
    terrainview.cpp \
    terrainmodel.cpp

HEADERS += aboutdialog.h \
    abstractobjecttool.h \
    abstracttiletool.h \
    abstracttool.h \
    addremovelayer.h \
    addremovemapobject.h \
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
    mapreaderinterface.h \
    mapscene.h \
    mapview.h \
    mapwriterinterface.h \
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
    saveasimagedialog.h \
    selectionrectangle.h \
    stampbrush.h \
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
    zoomable.h \
    terrainbrush.h \
    terraindock.h \
    terrainview.h \
    terrainmodel.h

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
    newimagelayerdialog.ui

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
