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
    automapper.cpp \
    automapperwrapper.cpp \
    automappingmanager.cpp \
    brushitem.cpp \
    documentmanager.cpp \
    filesystemwatcher.cpp \
    languagemanager.cpp \
    layerdock.cpp \
    layermodel.cpp \
    main.cpp \
    mainwindow.cpp \
    mapdocument.cpp \
    mapdocumentactionhandler.cpp \
    mapobjectitem.cpp \
    mapscene.cpp \
    mapview.cpp \
    painttilelayer.cpp \
    pluginmanager.cpp \
    preferencesdialog.cpp \
    preferences.cpp \
    propertiesdialog.cpp \
    propertiesmodel.cpp \
    resizehelper.cpp \
    resizedialog.cpp \
    tileselectionitem.cpp \
    tilesetdock.cpp \
    tilesetmanager.cpp \
    tilesetmodel.cpp \
    tilesetview.cpp \
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
    movemapobjecttogroup.cpp \
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
    tileselectiontool.cpp \
    abstracttiletool.cpp \
    abstracttool.cpp \
    changetileselection.cpp \
    clipboardmanager.cpp \
    offsetlayer.cpp \
    offsetmapdialog.cpp \
    bucketfilltool.cpp \
    filltiles.cpp \
    objectgrouppropertiesdialog.cpp \
    changeobjectgroupproperties.cpp \
    zoomable.cpp \
    addremovetileset.cpp \
    movetileset.cpp \
    createobjecttool.cpp \
    quickstampmanager.cpp \
    objectselectiontool.cpp \
    commandbutton.cpp \
    commanddatamodel.cpp \
    commanddialog.cpp \
    tiledapplication.cpp \
    command.cpp \
    abstractobjecttool.cpp \
    changepolygon.cpp \
    editpolygontool.cpp \
    selectionrectangle.cpp \
    objecttypes.cpp \
    objecttypesmodel.cpp \
    commandlineparser.cpp

HEADERS += aboutdialog.h \
    automapper.h \
    automapperwrapper.h \
    automappingmanager.h \
    brushitem.h \
    documentmanager.h \
    filesystemwatcher.h \
    languagemanager.h \
    layerdock.h \
    layermodel.h \
    mainwindow.h \
    mapreaderinterface.h \
    mapwriterinterface.h \
    mapdocument.h \
    mapdocumentactionhandler.h \
    mapobjectitem.h \
    mapscene.h \
    mapview.h \
    painttilelayer.h \
    pluginmanager.h \
    preferencesdialog.h \
    preferences.h \
    propertiesdialog.h \
    propertiesmodel.h \
    resizedialog.h \
    resizehelper.h \
    tileselectionitem.h \
    tilesetdock.h \
    tilesetmanager.h \
    tilesetmodel.h \
    tilesetview.h \
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
    movemapobjecttogroup.h \
    resizemapobject.h \
    addremovemapobject.h \
    addremovelayer.h \
    propertiesview.h \
    renamelayer.h \
    resizelayer.h \
    resizemap.h \
    objectpropertiesdialog.h \
    changemapobject.h \
    abstracttool.h \
    stampbrush.h \
    toolmanager.h \
    eraser.h \
    erasetiles.h \
    saveasimagedialog.h \
    utils.h \
    colorbutton.h \
    undodock.h \
    tileselectiontool.h \
    abstracttiletool.h \
    changetileselection.h \
    clipboardmanager.h \
    undocommands.h \
    offsetlayer.h \
    offsetmapdialog.h \
    bucketfilltool.h \
    filltiles.h \
    objectgrouppropertiesdialog.h \
    changeobjectgroupproperties.h \
    zoomable.h \
    addremovetileset.h \
    movetileset.h \
    createobjecttool.h \
    quickstampmanager.h \
    objectselectiontool.h \
    commanddatamodel.h \
    commanddialog.h \
    commandbutton.h \
    tiledapplication.h \
    command.h \
    abstractobjecttool.h \
    changepolygon.h \
    editpolygontool.h \
    selectionrectangle.h \
    rangeset.h \
    objecttypes.h \
    objecttypesmodel.h \
    commandlineparser.h

FORMS += aboutdialog.ui \
    mainwindow.ui \
    resizedialog.ui \
    preferencesdialog.ui \
    propertiesdialog.ui \
    newmapdialog.ui \
    newtilesetdialog.ui \
    saveasimagedialog.ui \
    offsetmapdialog.ui \
    objectpropertiesdialog.ui \
    commanddialog.ui
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
