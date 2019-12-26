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

SOURCES += dialogs/aboutdialog.cpp \
    tools/abstractobjecttool.cpp \
    tools/abstracttileselectiontool.cpp \
    tools/abstracttiletool.cpp \
    tools/abstracttilefilltool.cpp \
    tools/abstracttool.cpp \
    actionmanager.cpp \
    dialogs/addpropertydialog.cpp \
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
    brokenlinks.cpp \
    tools/brushitem.cpp \
    tools/bucketfilltool.cpp \
    capturestamphelper.cpp \
    changeimagelayerproperties.cpp \
    changelayer.cpp \
    changemapobject.cpp \
    changemapobjectsorder.cpp \
    changemapproperty.cpp \
    changeobjectgroupproperties.cpp \
    changepolygon.cpp \
    changeproperties.cpp \
    changeselectedarea.cpp \
    changeterrain.cpp \
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
    issuescounter.cpp \
    issuesdock.cpp \
    issuesmodel.cpp \
    clipboardmanager.cpp \
    colorbutton.cpp \
    commandbutton.cpp \
    command.cpp \
    commanddatamodel.cpp \
    dialogs/commanddialog.cpp \
    commandlineparser.cpp \
    commandmanager.cpp \
    consoledock.cpp \
    tools/createellipseobjecttool.cpp \
    tools/createobjecttool.cpp \
    tools/createpointobjecttool.cpp \
    tools/createpolygonobjecttool.cpp \
    tools/createrectangleobjecttool.cpp \
    tools/createscalableobjecttool.cpp \
    tools/createtemplatetool.cpp \
    tools/createtextobjecttool.cpp \
    tools/createtileobjecttool.cpp \
    document.cpp \
    documentmanager.cpp \
    dialogs/donationdialog.cpp \
    scripting/editableasset.cpp \
    scripting/editablegrouplayer.cpp \
    scripting/editableimagelayer.cpp \
    scripting/editablelayer.cpp \
    scripting/editablemanager.cpp \
    scripting/editablemap.cpp \
    scripting/editablemapobject.cpp \
    scripting/editableobject.cpp \
    scripting/editableobjectgroup.cpp \
    scripting/editableselectedarea.cpp \
    scripting/editableterrain.cpp \
    scripting/editabletile.cpp \
    scripting/editabletilelayer.cpp \
    scripting/editabletileset.cpp \
    editor.cpp \
    tools/editpolygontool.cpp \
    tools/eraser.cpp \
    tools/erasetiles.cpp \
    dialogs/exportasimagedialog.cpp \
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
    tools/layeroffsettool.cpp \
    tools/magicwandtool.cpp \
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
    minimaprenderer.cpp \
    movelayer.cpp \
    movemapobject.cpp \
    movemapobjecttogroup.cpp \
    moveterrain.cpp \
    dialogs/newmapdialog.cpp \
    newsbutton.cpp \
    newsfeed.cpp \
    dialogs/newtilesetdialog.cpp \
    newversionbutton.cpp \
    newversionchecker.cpp \
    dialogs/newversiondialog.cpp \
    noeditorwidget.cpp \
    objectgroupitem.cpp \
    objectsdock.cpp \
    objectselectionitem.cpp \
    tools/objectselectiontool.cpp \
    objectsview.cpp \
    objecttemplatemodel.cpp \
    objecttypeseditor.cpp \
    objecttypesmodel.cpp \
    offsetlayer.cpp \
    dialogs/offsetmapdialog.cpp \
    painttilelayer.cpp \
    pluginlistmodel.cpp \
    pointhandle.cpp \
    preferences.cpp \
    project.cpp \
    projectdock.cpp \
    projectmodel.cpp \
    dialogs/preferencesdialog.cpp \
    propertiesdock.cpp \
    propertybrowser.cpp \
    raiselowerhelper.cpp \
    regionvaluetype.cpp \
    renamewangset.cpp \
    reparentlayers.cpp \
    replacetemplate.cpp \
    replacetileset.cpp \
    dialogs/resizedialog.cpp \
    resizehelper.cpp \
    resizemap.cpp \
    resizemapobject.cpp \
    resizetilelayer.cpp \
    reversingproxymodel.cpp \
    rotatemapobject.cpp \
    scripting/scriptedaction.cpp \
    scripting/scriptedfileformat.cpp \
    scripting/scriptedtool.cpp \
    scripting/scriptfile.cpp \
    scripting/scriptmanager.cpp \
    scripting/scriptmodule.cpp \
    selectionrectangle.cpp \
    tools/selectsametiletool.cpp \
    session.cpp \
    tools/shapefilltool.cpp \
    shortcutsettingspage.cpp \
    snaphelper.cpp \
    stampactions.cpp \
    tools/stampbrush.cpp \
    stylehelper.cpp \
    swaptiles.cpp \
    templatesdock.cpp \
    tools/terrainbrush.cpp \
    terraindock.cpp \
    terrainmodel.cpp \
    terrainview.cpp \
    dialogs/texteditordialog.cpp \
    textpropertyedit.cpp \
    tileanimationeditor.cpp \
    tilecollisiondock.cpp \
    tiledapplication.cpp \
    tiledproxystyle.cpp \
    tilelayeredit.cpp \
    tilelayeritem.cpp \
    tilepainter.cpp \
    tileselectionitem.cpp \
    tools/tileselectiontool.cpp \
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
    wangtemplateview.cpp \
    wangtemplatemodel.cpp \
    zoomable.cpp

HEADERS += dialogs/aboutdialog.h \
    tools/abstractobjecttool.h \
    tools/abstracttileselectiontool.h \
    tools/abstracttiletool.h \
    tools/abstracttilefilltool.h \
    tools/abstracttool.h \
    actionmanager.h \
    dialogs/addpropertydialog.h \
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
    brokenlinks.h \
    tools/brushitem.h \
    tools/bucketfilltool.h \
    capturestamphelper.h \
    changeevents.h \
    changeimagelayerproperties.h \
    changelayer.h \
    changemapobject.h \
    changemapobjectsorder.h \
    changemapproperty.h \
    changeobjectgroupproperties.h \
    changepolygon.h \
    changeproperties.h \
    changeselectedarea.h \
    changeterrain.h \
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
    issuescounter.h \
    issuesdock.h \
    issuesmodel.h \
    clipboardmanager.h \
    colorbutton.h \
    commandbutton.h \
    commanddatamodel.h \
    dialogs/commanddialog.h \
    command.h \
    commandlineparser.h \
    commandmanager.h \
    consoledock.h \
    tools/createellipseobjecttool.h \
    tools/createobjecttool.h \
    tools/createpointobjecttool.h \
    tools/createpolygonobjecttool.h \
    tools/createrectangleobjecttool.h \
    tools/createscalableobjecttool.h \
    tools/createtemplatetool.h \
    tools/createtextobjecttool.h \
    tools/createtileobjecttool.h \
    document.h \
    documentmanager.h \
    dialogs/donationdialog.h \
    scripting/editableasset.h \
    scripting/editablegrouplayer.h \
    scripting/editableimagelayer.h \
    scripting/editablelayer.h \
    scripting/editablemanager.h \
    scripting/editablemap.h \
    scripting/editablemapobject.h \
    scripting/editableobject.h \
    scripting/editableobjectgroup.h \
    scripting/editableselectedarea.h \
    scripting/editableterrain.h \
    scripting/editabletile.h \
    scripting/editabletilelayer.h \
    scripting/editabletileset.h \
    editor.h \
    tools/editpolygontool.h \
    tools/eraser.h \
    tools/erasetiles.h \
    dialogs/exportasimagedialog.h \
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
    tools/layeroffsettool.h \
    macsupport.h \
    tools/magicwandtool.h \
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
    minimaprenderer.h \
    movelayer.h \
    movemapobject.h \
    movemapobjecttogroup.h \
    moveterrain.h \
    dialogs/newmapdialog.h \
    newsbutton.h \
    newsfeed.h \
    dialogs/newtilesetdialog.h \
    newversionbutton.h \
    newversionchecker.h \
    dialogs/newversiondialog.h \
    noeditorwidget.h \
    objectgroupitem.h \
    objectsdock.h \
    objectselectionitem.h \
    objecttemplatemodel.h \
    tools/objectselectiontool.h \
    objectsview.h \
    objecttypeseditor.h \
    objecttypesmodel.h \
    offsetlayer.h \
    dialogs/offsetmapdialog.h \
    painttilelayer.h \
    pluginlistmodel.h \
    pointhandle.h \
    dialogs/preferencesdialog.h \
    preferences.h \
    project.h \
    projectdock.h \
    projectmodel.h \
    propertiesdock.h \
    propertybrowser.h \
    raiselowerhelper.h \
    randompicker.h \
    rangeset.h \
    regionvaluetype.h \
    renamewangset.h \
    reparentlayers.h \
    replacetemplate.h \
    replacetileset.h \
    dialogs/resizedialog.h \
    resizehelper.h \
    resizemap.h \
    resizemapobject.h \
    resizetilelayer.h \
    reversingproxymodel.h \
    rotatemapobject.h \
    scripting/scriptedaction.h \
    scripting/scriptedfileformat.h \
    scripting/scriptedtool.h \
    scripting/scriptfile.h \
    scripting/scriptmanager.h \
    scripting/scriptmodule.h \
    selectionrectangle.h \
    tools/selectsametiletool.h \
    session.h \
    tools/shapefilltool.h \
    shortcutsettingspage.h \
    snaphelper.h \
    stampactions.h \
    tools/stampbrush.h \
    stylehelper.h \
    swaptiles.h \
    templatesdock.h \
    tools/terrainbrush.h \
    terraindock.h \
    terrainmodel.h \
    terrainview.h \
    dialogs/texteditordialog.h \
    textpropertyedit.h \
    tileanimationeditor.h \
    tilecollisiondock.h \
    tiledapplication.h \
    tiledproxystyle.h \
    tilelayeredit.h \
    tilelayeritem.h \
    tilepainter.h \
    tileselectionitem.h \
    tools/tileselectiontool.h \
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

FORMS += dialogs/aboutdialog.ui \
    dialogs/addpropertydialog.ui \
    dialogs/commanddialog.ui \
    dialogs/donationdialog.ui \
    dialogs/exportasimagedialog.ui \
    imagecolorpickerwidget.ui \
    mainwindow.ui \
    dialogs/newmapdialog.ui \
    dialogs/newtilesetdialog.ui \
    dialogs/newversiondialog.ui \
    noeditorwidget.ui \
    objecttypeseditor.ui \
    dialogs/offsetmapdialog.ui \
    dialogs/preferencesdialog.ui \
    dialogs/resizedialog.ui \
    shortcutsettingspage.ui \
    dialogs/texteditordialog.ui \
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
