include(../../tiled.pri)
include(../libtiled/libtiled.pri)
include(../qtpropertybrowser/src/qtpropertybrowser.pri)

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
    QMAKE_LIBDIR = $$OUT_PWD/../../lib $$QMAKE_LIBDIR
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
    addremovetiles.cpp \
    addremovetileset.cpp \
    automapper.cpp \
    automapperwrapper.cpp \
    automappingmanager.cpp \
    automappingutils.cpp  \
    brushitem.cpp \
    bucketfilltool.cpp \
    changeimagelayerposition.cpp \
    changeimagelayerproperties.cpp \
    changelayer.cpp \
    changemapobject.cpp \
    changemapobjectsorder.cpp \
    changemapproperties.cpp \
    changeobjectgroupproperties.cpp \
    changepolygon.cpp \
    changeproperties.cpp \
    changetileanimation.cpp \
    changetileobjectgroup.cpp \
    changetileselection.cpp \
    changetileterrain.cpp \
    clipboardmanager.cpp \
    colorbutton.cpp \
    commandbutton.cpp \
    command.cpp \
    commanddatamodel.cpp \
    commanddialog.cpp \
    commandlineparser.cpp \
    consoledock.cpp \
    createobjecttool.cpp \
    documentmanager.cpp \
    editpolygontool.cpp \
    editterraindialog.cpp \
    eraser.cpp \
    erasetiles.cpp \
    fileedit.cpp \
    filesystemwatcher.cpp \
    filltiles.cpp \
    flipmapobjects.cpp \
    geometry.cpp \
    imagelayeritem.cpp \
    languagemanager.cpp \
    layerdock.cpp \
    layermodel.cpp \
    main.cpp \
    mainwindow.cpp \
    mapdocumentactionhandler.cpp \
    mapdocument.cpp \
    mapobjectitem.cpp \
    mapobjectmodel.cpp \
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
    propertiesdock.cpp \
    propertybrowser.cpp \
    quickstampmanager.cpp \
    raiselowerhelper.cpp \
    renamelayer.cpp \
    renameterrain.cpp \
    resizedialog.cpp \
    resizehelper.cpp \
    resizemap.cpp \
    resizemapobject.cpp \
    resizetilelayer.cpp \
    rotatemapobject.cpp \
    saveasimagedialog.cpp \
    selectionrectangle.cpp \
    stampbrush.cpp \
    terrainbrush.cpp \
    terraindock.cpp \
    terrainmodel.cpp \
    terrainview.cpp \
    tileanimationdriver.cpp \
    tileanimationeditor.cpp \
    tilecollisioneditor.cpp \
    tiledapplication.cpp \
    tilelayeritem.cpp \
    tilepainter.cpp \
    tileselectionitem.cpp \
    tileselectiontool.cpp \
    tilesetchanges.cpp \
    tilesetdock.cpp \
    tilesetmanager.cpp \
    tilesetmodel.cpp \
    tilesetview.cpp \
    tmxmapreader.cpp \
    tmxmapwriter.cpp \
    toolmanager.cpp \
    undodock.cpp \
    utils.cpp \
    varianteditorfactory.cpp \
    variantpropertymanager.cpp \
    zoomable.cpp

HEADERS += aboutdialog.h \
    abstractobjecttool.h \
    abstracttiletool.h \
    abstracttool.h \
    addremovelayer.h \
    addremovemapobject.h \
    addremoveterrain.h \
    addremovetiles.h \
    addremovetileset.h \
    automapper.h \
    automapperwrapper.h \
    automappingmanager.h \
    automappingutils.h \
    brushitem.h \
    bucketfilltool.h \
    changeimagelayerposition.h \
    changeimagelayerproperties.h \
    changelayer.h \
    changemapobject.h \
    changemapobjectsorder.h \
    changemapproperties.h \
    changeobjectgroupproperties.h \
    changepolygon.h \
    changeproperties.h \
    changetileanimation.h \
    changetileobjectgroup.h \
    changetileselection.h \
    changetileterrain.h \
    clipboardmanager.h \
    colorbutton.h \
    commandbutton.h \
    commanddatamodel.h \
    commanddialog.h \
    command.h \
    commandlineparser.h \
    consoledock.h \
    createobjecttool.h \
    documentmanager.h \
    editpolygontool.h \
    editterraindialog.h \
    eraser.h \
    erasetiles.h \
    fileedit.h \
    filesystemwatcher.h \
    filltiles.h \
    flipmapobjects.h \
    geometry.h \
    imagelayeritem.h \
    languagemanager.h \
    layerdock.h \
    layermodel.h \
    macsupport.h \
    mainwindow.h \
    mapdocumentactionhandler.h \
    mapdocument.h \
    mapobjectitem.h \
    mapobjectmodel.h \
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
    propertiesdock.h \
    propertybrowser.h \
    quickstampmanager.h \
    raiselowerhelper.h \
    rangeset.h \
    renamelayer.h \
    renameterrain.h \
    resizedialog.h \
    resizehelper.h \
    resizemap.h \
    resizemapobject.h \
    resizetilelayer.h \
    rotatemapobject.h \
    saveasimagedialog.h \
    selectionrectangle.h \
    stampbrush.h \
    terrainbrush.h \
    terraindock.h \
    terrainmodel.h \
    terrainview.h \
    tileanimationdriver.h \
    tileanimationeditor.h \
    tilecollisioneditor.h \
    tiledapplication.h \
    tilelayeritem.h \
    tilepainter.h \
    tileselectionitem.h \
    tileselectiontool.h \
    tilesetchanges.h \
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
    varianteditorfactory.h \
    variantpropertymanager.h \
    zoomable.h

macx {
    OBJECTIVE_SOURCES += macsupport.mm
}

FORMS += aboutdialog.ui \
    commanddialog.ui \
    editterraindialog.ui \
    mainwindow.ui \
    newmapdialog.ui \
    newtilesetdialog.ui \
    offsetmapdialog.ui \
    preferencesdialog.ui \
    resizedialog.ui \
    saveasimagedialog.ui \
    tileanimationeditor.ui

icon32.path = $${PREFIX}/share/icons/hicolor/32x32/apps/
icon32.files += images/32x32/tiled.png
INSTALLS += icon32

icon16.path = $${PREFIX}/share/icons/hicolor/16x16/apps/
icon16.files += images/16x16/tiled.png
INSTALLS += icon16

iconscalable.path = $${PREFIX}/share/icons/hicolor/scalable/apps/
iconscalable.files += images/scalable/tiled.svg
INSTALLS += iconscalable

mimeicon16.path = $${PREFIX}/share/icons/hicolor/16x16/mimetypes/
mimeicon16.files += images/16x16/application-x-tiled.png
INSTALLS += mimeicon16

mimeicon32.path = $${PREFIX}/share/icons/hicolor/32x32/mimetypes/
mimeicon32.files += images/32x32/application-x-tiled.png
INSTALLS += mimeicon32

mimeiconscalable.path = $${PREFIX}/share/icons/hicolor/scalable/mimetypes/
mimeiconscalable.files += images/scalable/application-x-tiled.svg
INSTALLS += mimeiconscalable

mimeinfofile.path = $${PREFIX}/share/mime/packages/
mimeinfofile.files += ../../docs/mime/tiled.xml
INSTALLS += mimeinfofile

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
