include(../../tiled.pri)
include(../libtiled/libtiled.pri)
include(../qtpropertybrowser/src/qtpropertybrowser.pri)
include(../qtsingleapplication/src/qtsingleapplication.pri)

TEMPLATE = app
TARGET = tiled
target.path = $${PREFIX}/bin
INSTALLS += target
win32 {
    DESTDIR = ../..
} else {
    DESTDIR = ../../bin
}

QT += widgets

contains(QT_CONFIG, opengl):!macx:!minQtVersion(5, 4, 0) {
    QT += opengl
}

DEFINES += TILED_VERSION=$${TILED_VERSION}

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII

macx {
    QMAKE_LIBDIR += $$OUT_PWD/../../bin/Tiled.app/Contents/Frameworks
    LIBS += -framework Foundation
    DEFINES += QT_NO_OPENGL
    OBJECTIVE_SOURCES += macsupport.mm

    sparkle {
        SPARKLE_DIR = /Library/Frameworks

        !exists($${SPARKLE_DIR}/Sparkle.framework) {
            error("Sparkle.framework not found at $${SPARKLE_DIR}")
        }

        DEFINES += TILED_SPARKLE
        LIBS += -framework Sparkle -framework AppKit
        LIBS += -F$${SPARKLE_DIR}
        QMAKE_OBJECTIVE_CFLAGS += -F$${SPARKLE_DIR}
        OBJECTIVE_SOURCES += sparkleautoupdater.mm

        APP_RESOURCES.path = Contents/Resources
        APP_RESOURCES.files = \
            ../../dist/dsa_pub.pem

        SPARKLE_FRAMEWORK.path = Contents/Frameworks
        SPARKLE_FRAMEWORK.files = $${SPARKLE_DIR}/Sparkle.framework

        QMAKE_BUNDLE_DATA += APP_RESOURCES SPARKLE_FRAMEWORK
    }
} else:win32 {
    LIBS += -L$$OUT_PWD/../../lib
} else {
    QMAKE_LIBDIR = $$OUT_PWD/../../lib $$QMAKE_LIBDIR
}

# Make sure the Tiled executable can find libtiled
!win32:!macx:!cygwin:contains(RPATH, yes) {
    QMAKE_RPATHDIR += \$\$ORIGIN/../lib

    # It is not possible to use ORIGIN in QMAKE_RPATHDIR, so a bit manually
    QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,$$join(QMAKE_RPATHDIR, ":")\'
    QMAKE_RPATHDIR =
}

SOURCES += aboutdialog.cpp \
    abstractobjecttool.cpp \
    abstracttileselectiontool.cpp \
    abstracttiletool.cpp \
    abstracttilefilltool.cpp \
    abstracttool.cpp \
    actionmanager.cpp \
    addpropertydialog.cpp \
    addremovelayer.cpp \
    addremovemapobject.cpp \
    addremoveterrain.cpp \
    addremovetiles.cpp \
    addremovetileset.cpp \
    addremovewangset.cpp \
    adjusttileindexes.cpp \
    automapper.cpp \
    automapperwrapper.cpp \
    automappingmanager.cpp \
    automappingutils.cpp  \
    autoupdater.cpp \
    brokenlinks.cpp \
    brushitem.cpp \
    bucketfilltool.cpp \
    capturestamphelper.cpp \
    changeimagelayerposition.cpp \
    changeimagelayerproperties.cpp \
    changelayer.cpp \
    changemapobject.cpp \
    changemapobjectsorder.cpp \
    changemapproperty.cpp \
    changeobjectgroupproperties.cpp \
    changepolygon.cpp \
    changeproperties.cpp \
    changeselectedarea.cpp \
    changetile.cpp \
    changetileanimation.cpp \
    changetileimagesource.cpp \
    changetileobjectgroup.cpp \
    changetileprobability.cpp \
    changetileterrain.cpp \
    changetilewangid.cpp \
    changewangcolordata.cpp \
    changewangsetdata.cpp \
    clickablelabel.cpp \
    clipboardmanager.cpp \
    colorbutton.cpp \
    commandbutton.cpp \
    command.cpp \
    commanddatamodel.cpp \
    commanddialog.cpp \
    commandlineparser.cpp \
    commandmanager.cpp \
    consoledock.cpp \
    createellipseobjecttool.cpp \
    createmultipointobjecttool.cpp \
    createobjecttool.cpp \
    createpointobjecttool.cpp \
    createpolygonobjecttool.cpp \
    createpolylineobjecttool.cpp \
    createrectangleobjecttool.cpp \
    createscalableobjecttool.cpp \
    createtemplatetool.cpp \
    createtextobjecttool.cpp \
    createtileobjecttool.cpp \
    document.cpp \
    documentmanager.cpp \
    editor.cpp \
    editpolygontool.cpp \
    eraser.cpp \
    erasetiles.cpp \
    exportasimagedialog.cpp \
    filechangedwarning.cpp \
    fileedit.cpp \
    flexiblescrollbar.cpp \
    flipmapobjects.cpp \
    geometry.cpp \
    grouplayeritem.cpp \
    iconcheckdelegate.cpp \
    id.cpp \
    imagecolorpickerwidget.cpp \
    imagelayeritem.cpp \
    languagemanager.cpp \
    layerdock.cpp \
    layeritem.cpp \
    layermodel.cpp \
    layeroffsettool.cpp \
    magicwandtool.cpp \
    main.cpp \
    maintoolbar.cpp \
    mainwindow.cpp \
    mapdocumentactionhandler.cpp \
    mapdocument.cpp \
    mapeditor.cpp \
    mapitem.cpp \
    mapobjectitem.cpp \
    mapobjectmodel.cpp \
    mapscene.cpp \
    mapsdock.cpp \
    mapview.cpp \
    minimap.cpp \
    minimapdock.cpp \
    minimaprenderer.cpp \
    movelayer.cpp \
    movemapobject.cpp \
    movemapobjecttogroup.cpp \
    moveterrain.cpp \
    newmapdialog.cpp \
    newtilesetdialog.cpp \
    noeditorwidget.cpp \
    objectgroupitem.cpp \
    objectsdock.cpp \
    objectselectionitem.cpp \
    objectselectiontool.cpp \
    objecttemplatemodel.cpp \
    objecttypeseditor.cpp \
    objecttypesmodel.cpp \
    offsetlayer.cpp \
    offsetmapdialog.cpp \
    painttilelayer.cpp \
    patreondialog.cpp \
    pluginlistmodel.cpp \
    preferences.cpp \
    preferencesdialog.cpp \
    propertiesdock.cpp \
    propertybrowser.cpp \
    raiselowerhelper.cpp \
    renamelayer.cpp \
    renameterrain.cpp \
    renamewangset.cpp \
    reparentlayers.cpp \
    replacetemplate.cpp \
    replacetileset.cpp \
    resizedialog.cpp \
    resizehelper.cpp \
    resizemap.cpp \
    resizemapobject.cpp \
    resizetilelayer.cpp \
    reversingproxymodel.cpp \
    rotatemapobject.cpp \
    selectionrectangle.cpp \
    selectsametiletool.cpp \
    shapefilltool.cpp \
    snaphelper.cpp \
    stampactions.cpp \
    stampbrush.cpp \
    standardautoupdater.cpp \
    stylehelper.cpp \
    swaptiles.cpp \
    templatesdock.cpp \
    terrainbrush.cpp \
    terraindock.cpp \
    terrainmodel.cpp \
    terrainview.cpp \
    texteditordialog.cpp \
    textpropertyedit.cpp \
    tileanimationeditor.cpp \
    tilecollisiondock.cpp \
    tiledapplication.cpp \
    tiledproxystyle.cpp \
    tilelayeritem.cpp \
    tilepainter.cpp \
    tileselectionitem.cpp \
    tileselectiontool.cpp \
    tilesetchanges.cpp \
    tilesetdock.cpp \
    tilesetdocument.cpp \
    tilesetdocumentsmodel.cpp \
    tileseteditor.cpp \
    tilesetmodel.cpp \
    tilesetparametersedit.cpp \
    tilesetterrainmodel.cpp \
    tilesetwangsetmodel.cpp \
    tilesetview.cpp \
    tilestamp.cpp \
    tilestampmanager.cpp \
    tilestampmodel.cpp \
    tilestampsdock.cpp \
    tmxmapformat.cpp \
    toolmanager.cpp \
    treeviewcombobox.cpp \
    undodock.cpp \
    utils.cpp \
    varianteditorfactory.cpp \
    variantpropertymanager.cpp \
    wangbrush.cpp \
    wangcolormodel.cpp \
    wangcolorview.cpp \
    wangsetview.cpp \
    wangsetmodel.cpp \
    wangdock.cpp \
    wangfiller.cpp \
    wangtemplateview.cpp \
    wangtemplatemodel.cpp \
    zoomable.cpp

HEADERS += aboutdialog.h \
    abstractobjecttool.h \
    abstracttileselectiontool.h \
    abstracttiletool.h \
    abstracttilefilltool.h \
    abstracttool.h \
    actionmanager.h \
    addpropertydialog.h \
    addremovelayer.h \
    addremovemapobject.h \
    addremoveterrain.h \
    addremovetileset.h \
    addremovetiles.h \
    addremovewangset.h \
    adjusttileindexes.h \
    automapper.h \
    automapperwrapper.h \
    automappingmanager.h \
    automappingutils.h \
    autoupdater.h \
    brokenlinks.h \
    brushitem.h \
    bucketfilltool.h \
    capturestamphelper.h \
    changeimagelayerposition.h \
    changeimagelayerproperties.h \
    changelayer.h \
    changemapobject.h \
    changemapobjectsorder.h \
    changemapproperty.h \
    changeobjectgroupproperties.h \
    changepolygon.h \
    changeproperties.h \
    changeselectedarea.h \
    changetile.h \
    changetileanimation.h \
    changetileimagesource.h \
    changetileobjectgroup.h \
    changetileprobability.h \
    changetileterrain.h \
    changetilewangid.h \
    changewangcolordata.h \
    changewangsetdata.h \
    clickablelabel.h \
    clipboardmanager.h \
    colorbutton.h \
    commandbutton.h \
    commanddatamodel.h \
    commanddialog.h \
    command.h \
    commandlineparser.h \
    commandmanager.h \
    consoledock.h \
    containerhelpers.h \
    createellipseobjecttool.h \
    createmultipointobjecttool.h \
    createobjecttool.h \
    createpointobjecttool.h \
    createpolygonobjecttool.h \
    createpolylineobjecttool.h \
    createrectangleobjecttool.h \
    createscalableobjecttool.h \
    createtemplatetool.h \
    createtextobjecttool.h \
    createtileobjecttool.h \
    document.h \
    documentmanager.h \
    editor.h \
    editpolygontool.h \
    eraser.h \
    erasetiles.h \
    exportasimagedialog.h \
    filechangedwarning.h \
    fileedit.h \
    flexiblescrollbar.h \
    flipmapobjects.h \
    geometry.h \
    grouplayeritem.h \
    iconcheckdelegate.h \
    id.h \
    imagecolorpickerwidget.h \
    imagelayeritem.h \
    languagemanager.h \
    layerdock.h \
    layeritem.h \
    layermodel.h \
    layeroffsettool.h \
    macsupport.h \
    magicwandtool.h \
    maintoolbar.h \
    mainwindow.h \
    mapdocumentactionhandler.h \
    mapdocument.h \
    mapeditor.h \
    mapitem.h \
    mapobjectitem.h \
    mapobjectmodel.h \
    mapscene.h \
    mapsdock.h \
    mapview.h \
    minimapdock.h \
    minimap.h \
    minimaprenderer.h \
    movelayer.h \
    movemapobject.h \
    movemapobjecttogroup.h \
    moveterrain.h \
    newmapdialog.h \
    newtilesetdialog.h \
    noeditorwidget.h \
    objectgroupitem.h \
    objectsdock.h \
    objectselectionitem.h \
    objecttemplatemodel.h \
    objectselectiontool.h \
    objecttypeseditor.h \
    objecttypesmodel.h \
    offsetlayer.h \
    offsetmapdialog.h \
    painttilelayer.h \
    patreondialog.h \
    pluginlistmodel.h \
    preferencesdialog.h \
    preferences.h \
    propertiesdock.h \
    propertybrowser.h \
    raiselowerhelper.h \
    randompicker.h \
    rangeset.h \
    renamelayer.h \
    renameterrain.h \
    renamewangset.h \
    reparentlayers.h \
    replacetemplate.h \
    replacetileset.h \
    resizedialog.h \
    resizehelper.h \
    resizemap.h \
    resizemapobject.h \
    resizetilelayer.h \
    reversingproxymodel.h \
    rotatemapobject.h \
    selectionrectangle.h \
    selectsametiletool.h \
    shapefilltool.h \
    snaphelper.h \
    sparkleautoupdater.h \
    stampactions.h \
    stampbrush.h \
    standardautoupdater.h \
    stylehelper.h \
    swaptiles.h \
    templatesdock.h \
    terrainbrush.h \
    terraindock.h \
    terrainmodel.h \
    terrainview.h \
    texteditordialog.h \
    textpropertyedit.h \
    tileanimationeditor.h \
    tilecollisiondock.h \
    tiledapplication.h \
    tiledproxystyle.h \
    tilelayeritem.h \
    tilepainter.h \
    tileselectionitem.h \
    tileselectiontool.h \
    tilesetchanges.h \
    tilesetdock.h \
    tilesetdocument.h \
    tilesetdocumentsmodel.h \
    tileseteditor.h \
    tilesetmodel.h \
    tilesetparametersedit.h \
    tilesetterrainmodel.h \
    tilesetwangsetmodel.h \
    tilesetview.h \
    tilestamp.h \
    tilestampmanager.h \
    tilestampmodel.h \
    tilestampsdock.h \
    tmxmapformat.h \
    toolmanager.h \
    treeviewcombobox.h \
    undocommands.h \
    undodock.h \
    utils.h \
    varianteditorfactory.h \
    variantpropertymanager.h \
    wangbrush.h \
    wangcolormodel.h \
    wangcolorview.h \
    wangsetview.h \
    wangsetmodel.h \
    wangdock.h \
    wangfiller.h \
    wangtemplateview.h \
    wangtemplatemodel.h \
    zoomable.h

FORMS += aboutdialog.ui \
    addpropertydialog.ui \
    commanddialog.ui \
    exportasimagedialog.ui \
    imagecolorpickerwidget.ui \
    mainwindow.ui \
    newmapdialog.ui \
    newtilesetdialog.ui \
    noeditorwidget.ui \
    objecttypeseditor.ui \
    offsetmapdialog.ui \
    patreondialog.ui \
    preferencesdialog.ui \
    resizedialog.ui \
    texteditordialog.ui \
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
mimeinfofile.files += ../../mime/tiled.xml
INSTALLS += mimeinfofile

thumbnailgenerator.path = $${PREFIX}/share/thumbnailers/
thumbnailgenerator.files += ../../mime/tiled.thumbnailer
INSTALLS += thumbnailgenerator

desktopfile.path = $${PREFIX}/share/applications/
desktopfile.files += ../../tiled.desktop
INSTALLS += desktopfile

appdatafile.path = $${PREFIX}/share/metainfo/
appdatafile.files += ../../tiled.appdata.xml
INSTALLS += appdatafile

manpage.path = $${PREFIX}/share/man/man1/
manpage.files += ../../man/tiled.1
INSTALLS += manpage

RESOURCES += tiled.qrc
macx {
    TARGET = Tiled
    QMAKE_INFO_PLIST = Info.plist
    QMAKE_ASSET_CATALOGS += images/tiled.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = tiled-icon-mac
}
win32 {
    RC_FILE = tiled.rc.in
    PRECOMPILED_HEADER = pch.h
}
win32:INCLUDEPATH += .
contains(CONFIG, static) {
    DEFINES += STATIC_BUILD
    QTPLUGIN += qgif \
        qjpeg \
        qtiff
}
