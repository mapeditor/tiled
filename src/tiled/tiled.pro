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

QT += widgets qml

contains(QT_CONFIG, opengl):minQtVersion(6, 0, 0) {
    QT += openglwidgets
}

!contains(DISABLE_DBUS, yes):contains(QT_CONFIG, dbus) {
    QT += dbus
    DEFINES += TILED_ENABLE_DBUS
}

DEFINES += TILED_VERSION=$${TILED_VERSION}

DEFINES += QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII

macx {
    QMAKE_LIBDIR += $$OUT_PWD/../../bin/Tiled.app/Contents/Frameworks
    LIBS += -framework Foundation
    DEFINES += QT_NO_OPENGL
    OBJECTIVE_SOURCES += macsupport.mm
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

# Support linking to system Zstandard install
unix:contains(SYSTEM_ZSTD, yes) {
    DEFINES += TILED_ZSTD_SUPPORT
}

SOURCES += aboutdialog.cpp \
    abstractobjecttool.cpp \
    abstracttileselectiontool.cpp \
    abstracttiletool.cpp \
    abstracttilefilltool.cpp \
    abstracttool.cpp \
    abstractworldtool.cpp \
    actionmanager.cpp \
    addpropertydialog.cpp \
    addremovelayer.cpp \
    addremovemapobject.cpp \
    addremovetiles.cpp \
    addremovetileset.cpp \
    addremovewangset.cpp \
    adjusttileindexes.cpp \
    automapper.cpp \
    automapperwrapper.cpp \
    automappingmanager.cpp \
    automappingutils.cpp  \
    brokenlinks.cpp \
    brushitem.cpp \
    bucketfilltool.cpp \
    capturestamphelper.cpp \
    changeimagelayerproperty.cpp \
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
    changetilewangid.cpp \
    changewangcolordata.cpp \
    changewangsetdata.cpp \
    clickablelabel.cpp \
    issuescounter.cpp \
    issuesdock.cpp \
    issuesmodel.cpp \
    clipboardmanager.cpp \
    colorbutton.cpp \
    commandbutton.cpp \
    command.cpp \
    commanddatamodel.cpp \
    commanddialog.cpp \
    commandlineparser.cpp \
    commandmanager.cpp \
    commandsedit.cpp \
    consoledock.cpp \
    createellipseobjecttool.cpp \
    createobjecttool.cpp \
    createpointobjecttool.cpp \
    createpolygonobjecttool.cpp \
    createrectangleobjecttool.cpp \
    createscalableobjecttool.cpp \
    createtemplatetool.cpp \
    createtextobjecttool.cpp \
    createtileobjecttool.cpp \
    custompropertieshelper.cpp \
    debugdrawitem.cpp \
    document.cpp \
    documentmanager.cpp \
    donationpopup.cpp \
    editableasset.cpp \
    editablegrouplayer.cpp \
    editableimagelayer.cpp \
    editablelayer.cpp \
    editablemanager.cpp \
    editablemap.cpp \
    editablemapobject.cpp \
    editableobject.cpp \
    editableobjectgroup.cpp \
    editableselectedarea.cpp \
    editabletile.cpp \
    editabletilelayer.cpp \
    editabletileset.cpp \
    editablewangset.cpp \
    editor.cpp \
    editpolygontool.cpp \
    eraser.cpp \
    erasetiles.cpp \
    exportasimagedialog.cpp \
    exporthelper.cpp \
    filechangedwarning.cpp \
    fileedit.cpp \
    filteredit.cpp \
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
    locatorwidget.cpp \
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
    mapview.cpp \
    minimap.cpp \
    minimapdock.cpp \
    movelayer.cpp \
    movemapobject.cpp \
    movemapobjecttogroup.cpp \
    newmapdialog.cpp \
    newsbutton.cpp \
    newsfeed.cpp \
    newtilesetdialog.cpp \
    newversionbutton.cpp \
    newversionchecker.cpp \
    newversiondialog.cpp \
    noeditorwidget.cpp \
    objectgroupitem.cpp \
    objectrefdialog.cpp \
    objectrefedit.cpp \
    objectreferenceitem.cpp \
    objectreferencetool.cpp \
    objectsdock.cpp \
    objectselectionitem.cpp \
    objectselectiontool.cpp \
    objectsview.cpp \
    objecttypeseditor.cpp \
    objecttypesmodel.cpp \
    offsetlayer.cpp \
    offsetmapdialog.cpp \
    painttilelayer.cpp \
    pluginlistmodel.cpp \
    pointhandle.cpp \
    popupwidget.cpp \
    preferences.cpp \
    preferencesdialog.cpp \
    project.cpp \
    projectdock.cpp \
    projectmanager.cpp \
    projectmodel.cpp \
    projectpropertiesdialog.cpp \
    propertiesdock.cpp \
    propertybrowser.cpp \
    propertytypeseditor.cpp \
    propertytypesmodel.cpp \
    raiselowerhelper.cpp \
    regionvaluetype.cpp \
    relocatetiles.cpp \
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
    scriptedaction.cpp \
    scriptedfileformat.cpp \
    scriptedtool.cpp \
    scriptfile.cpp \
    scriptfileformatwrappers.cpp \
    scriptfileinfo.cpp \
    scriptimage.cpp \
    scriptmanager.cpp \
    scriptmodule.cpp \
    scriptprocess.cpp \
    selectionrectangle.cpp \
    selectsametiletool.cpp \
    session.cpp \
    shapefilltool.cpp \
    shortcutsettingspage.cpp \
    snaphelper.cpp \
    stampactions.cpp \
    stampbrush.cpp \
    stylehelper.cpp \
    swaptiles.cpp \
    tabbar.cpp \
    templatesdock.cpp \
    texteditordialog.cpp \
    textpropertyedit.cpp \
    tileanimationeditor.cpp \
    tilecollisiondock.cpp \
    tiledapplication.cpp \
    tiledproxystyle.cpp \
    tilelayeredit.cpp \
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
    tilesetwangsetmodel.cpp \
    tilesetview.cpp \
    tilestamp.cpp \
    tilestampmanager.cpp \
    tilestampmodel.cpp \
    tilestampsdock.cpp \
    tmxmapformat.cpp \
    toolmanager.cpp \
    treeviewcombobox.cpp \
    undocommands.cpp \
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
    wangoverlay.cpp \
    wangtemplatemodel.cpp \
    wangtemplateview.cpp \
    worlddocument.cpp \
    worldmovemaptool.cpp \
    zoomable.cpp

HEADERS += aboutdialog.h \
    abstractobjecttool.h \
    abstracttileselectiontool.h \
    abstracttiletool.h \
    abstracttilefilltool.h \
    abstracttool.h \
    abstractworldtool.h \
    actionmanager.h \
    addpropertydialog.h \
    addremovelayer.h \
    addremovemapobject.h \
    addremovetileset.h \
    addremovetiles.h \
    addremovewangset.h \
    adjusttileindexes.h \
    automapper.h \
    automapperwrapper.h \
    automappingmanager.h \
    automappingutils.h \
    brokenlinks.h \
    brushitem.h \
    bucketfilltool.h \
    capturestamphelper.h \
    changeevents.h \
    changeimagelayerproperty.h \
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
    changetilewangid.h \
    changewangcolordata.h \
    changewangsetdata.h \
    clickablelabel.h \
    issuescounter.h \
    issuesdock.h \
    issuesmodel.h \
    clipboardmanager.h \
    colorbutton.h \
    commandbutton.h \
    commanddatamodel.h \
    commanddialog.h \
    command.h \
    commandlineparser.h \
    commandmanager.h \
    commandsedit.h \
    consoledock.h \
    createellipseobjecttool.h \
    createobjecttool.h \
    createpointobjecttool.h \
    createpolygonobjecttool.h \
    createrectangleobjecttool.h \
    createscalableobjecttool.h \
    createtemplatetool.h \
    createtextobjecttool.h \
    createtileobjecttool.h \
    custompropertieshelper.h \
    debugdrawitem.h \
    document.h \
    documentmanager.h \
    donationpopup.h \
    editableasset.h \
    editablegrouplayer.h \
    editableimagelayer.h \
    editablelayer.h \
    editablemanager.h \
    editablemap.h \
    editablemapobject.h \
    editableobject.h \
    editableobjectgroup.h \
    editableselectedarea.h \
    editabletile.h \
    editabletilelayer.h \
    editabletileset.h \
    editablewangset.h \
    editor.h \
    editpolygontool.h \
    eraser.h \
    erasetiles.h \
    exportasimagedialog.h \
    exporthelper.h \
    filechangedwarning.h \
    fileedit.h \
    filteredit.h \
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
    locatorwidget.h \
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
    mapview.h \
    minimapdock.h \
    minimap.h \
    movelayer.h \
    movemapobject.h \
    movemapobjecttogroup.h \
    newmapdialog.h \
    newsbutton.h \
    newsfeed.h \
    newtilesetdialog.h \
    newversionbutton.h \
    newversionchecker.h \
    newversiondialog.h \
    noeditorwidget.h \
    objectgroupitem.h \
    objectrefdialog.h \
    objectrefedit.h \
    objectreferenceitem.h \
    objectreferencetool.h \
    objectsdock.h \
    objectselectionitem.h \
    objectselectiontool.h \
    objectsview.h \
    objecttypeseditor.h \
    objecttypesmodel.h \
    offsetlayer.h \
    offsetmapdialog.h \
    painttilelayer.h \
    pluginlistmodel.h \
    pointhandle.h \
    popupwidget.h \
    preferences.h \
    preferencesdialog.h \
    project.h \
    projectdock.h \
    projectmanager.h \
    projectmodel.h \
    projectpropertiesdialog.h \
    propertiesdock.h \
    propertybrowser.h \
    propertytypeseditor.h \
    propertytypesmodel.h \
    raiselowerhelper.h \
    randompicker.h \
    rangeset.h \
    regionvaluetype.h \
    relocatetiles.h \
    reparentlayers.h \
    replacetemplate.h \
    replacetileset.h \
    resizedialog.h \
    resizehelper.h \
    resizemap.h \
    resizemapobject.h \
    resizetilelayer.h \
    reversingproxymodel.h \
    reversingrecursivefiltermodel.h \
    rotatemapobject.h \
    scriptedaction.h \
    scriptedfileformat.h \
    scriptedtool.h \
    scriptfile.h \
    scriptfileformatwrappers.h \
    scriptfileinfo.h \
    scriptimage.h \
    scriptmanager.h \
    scriptmodule.h \
    scriptprocess.h \
    selectionrectangle.h \
    selectsametiletool.h \
    session.h \
    shapefilltool.h \
    shortcutsettingspage.h \
    snaphelper.h \
    stampactions.h \
    stampbrush.h \
    stylehelper.h \
    swaptiles.h \
    tabbar.h \
    templatesdock.h \
    texteditordialog.h \
    textpropertyedit.h \
    tileanimationeditor.h \
    tilecollisiondock.h \
    tiledapplication.h \
    tiledproxystyle.h \
    tilelayeredit.h \
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
    wangoverlay.h \
    wangtemplatemodel.h \
    wangtemplateview.h \
    worlddocument.h \
    worldmovemaptool.h \
    zoomable.h

FORMS += aboutdialog.ui \
    addpropertydialog.ui \
    commanddialog.ui \
    commandsedit.ui \
    exportasimagedialog.ui \
    imagecolorpickerwidget.ui \
    mainwindow.ui \
    newmapdialog.ui \
    newtilesetdialog.ui \
    newversiondialog.ui \
    noeditorwidget.ui \
    objectrefdialog.ui \
    objecttypeseditor.ui \
    offsetmapdialog.ui \
    preferencesdialog.ui \
    projectpropertiesdialog.ui \
    propertytypeseditor.ui \
    resizedialog.ui \
    shortcutsettingspage.ui \
    texteditordialog.ui \
    tileanimationeditor.ui

icon32.path = $${PREFIX}/share/icons/hicolor/32x32/apps/
icon32.files += images/32/tiled.png
INSTALLS += icon32

icon16.path = $${PREFIX}/share/icons/hicolor/16x16/apps/
icon16.files += images/16/tiled.png
INSTALLS += icon16

iconscalable.path = $${PREFIX}/share/icons/hicolor/scalable/apps/
iconscalable.files += images/scalable/tiled.svg
INSTALLS += iconscalable

mimeicon16.path = $${PREFIX}/share/icons/hicolor/16x16/mimetypes/
mimeicon16.files += images/16/application-x-tiled.png
INSTALLS += mimeicon16

mimeicon32.path = $${PREFIX}/share/icons/hicolor/32x32/mimetypes/
mimeicon32.files += images/32/application-x-tiled.png
INSTALLS += mimeicon32

mimeiconscalable.path = $${PREFIX}/share/icons/hicolor/scalable/mimetypes/
mimeiconscalable.files += images/scalable/application-x-tiled.svg
INSTALLS += mimeiconscalable

mimeinfofile.path = $${PREFIX}/share/mime/packages/
mimeinfofile.files += ../../mime/org.mapeditor.Tiled.xml
INSTALLS += mimeinfofile

thumbnailgenerator.path = $${PREFIX}/share/thumbnailers/
thumbnailgenerator.files += ../../mime/tiled.thumbnailer
INSTALLS += thumbnailgenerator

desktopfile.path = $${PREFIX}/share/applications/
desktopfile.files += ../../org.mapeditor.Tiled.desktop
INSTALLS += desktopfile

appdatafile.path = $${PREFIX}/share/metainfo/
appdatafile.files += ../../org.mapeditor.Tiled.appdata.xml
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
