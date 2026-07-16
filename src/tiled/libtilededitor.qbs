import qbs.File
import qbs.FileInfo
import qbs.TextFile

DynamicLibrary {
    targetName: "tilededitor"

    Depends { name: "libtiled" }
    Depends { name: "translations" }
    Depends { name: "qtsingleapplication" }
    Depends { name: "Qt"; submodules: ["core", "widgets", "concurrent", "qml", "quick", "quickcontrols2", "quickwidgets"]; versionAtLeast: "6.2.0" }
    Depends { name: "Qt.svg"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt.openglwidgets"; required: false }
    Depends { name: "Qt.dbus"; condition: qbs.targetOS.contains("linux") && project.dbus; required: false }
    Depends { name: "Qt.gui-private"; condition: qbs.targetOS.contains("windows") }

    // When the build multiplexes over multiple architectures, the QML type
    // description only needs to be installed once, since its contents do not
    // depend on the architecture.
    readonly property bool installQmlTypeDescription: {
        return !qbs.architectures
                || qbs.architectures.length < 2
                || qbs.architecture === qbs.architectures[0];
    }

    // Generates the registration of the QML types provided for use by QML
    // extensions (see QML_NAMED_ELEMENT), along with a "plugins.qmltypes"
    // file describing them for use by tooling like qmllint and qmlls.
    Qt.qml.importName: "Tiled"
    Qt.qml.importVersion: "1.0"
    Qt.qml.typesInstallDir: installQmlTypeDescription ? "qml/Tiled" : undefined
    Qt.qml.extraMetaTypesFiles: qtMetaTypes.files

    // Locates the metatypes files of the used Qt modules, needed to resolve
    // the Qt base classes of the QML registered types. Qbs passes these
    // files automatically when they are in the archdata directory, which is
    // the case since Qt 6.5 (qtbase commit 4234ce12dc819). Older Qt builds
    // place them under lib/metatypes instead, a location Qbs does not
    // currently check, so this Probe supplies them in that case. It can be
    // removed entirely once the minimum Qbs version includes the upstream
    // fix "Qt support: Look for metatypes files in the libraries directory
    // too".
    Probe {
        id: qtMetaTypes
        property string libPath: Qt.core.libPath
        property string mkspecPath: Qt.core.mkspecPath
        property stringList files
        configure: {
            var list = [];

            // The used mkspec lives at <archdata>/mkspecs/<spec>, so the
            // archdata directory is two levels up from it.
            var archDataPath = FileInfo.path(FileInfo.path(mkspecPath));
            var archDataMetaTypesDir = FileInfo.joinPaths(archDataPath,
                                                          "metatypes");

            var handledByQbs = false;
            if (File.exists(archDataMetaTypesDir)) {
                var entries = File.directoryEntries(archDataMetaTypesDir,
                                                    File.Files);
                for (var i = 0; i < entries.length; ++i) {
                    if (entries[i].endsWith("_metatypes.json")) {
                        handledByQbs = true;
                        break;
                    }
                }
            }

            if (!handledByQbs) {
                var libMetaTypesDir = FileInfo.joinPaths(libPath, "metatypes");
                if (File.exists(libMetaTypesDir)) {
                    var libEntries = File.directoryEntries(libMetaTypesDir,
                                                           File.Files);
                    for (var j = 0; j < libEntries.length; ++j) {
                        if (libEntries[j].endsWith("_metatypes.json")) {
                            list.push(FileInfo.joinPaths(libMetaTypesDir,
                                                         libEntries[j]));
                        }
                    }
                }
            }

            files = list;
            found = true;
        }
    }

    Group {
        name: "QML type description"
        files: ["qml/Tiled/qmldir"]
        qbs.install: installQmlTypeDescription
        qbs.installDir: "qml/Tiled"
    }

    cpp.includePaths: {
        var paths = ["."];

        if (project.sentry)
            paths.push("../../sentry-native/install/include");

        return paths;
    }

    cpp.useCxxPrecompiledHeader: qbs.buildVariant != "debug"
    cpp.cxxLanguageVersion: "c++17"
    cpp.visibility: "minimal"

    cpp.defines: {
        var defs = [
            "TILED_EDITOR_LIBRARY",
            "TILED_VERSION=" + project.version,
            "QT_DISABLE_DEPRECATED_BEFORE=0x050F00",
            "QT_NO_DEPRECATED_WARNINGS",
            "QT_NO_CAST_FROM_ASCII",
            "QT_NO_CAST_TO_ASCII",
            "QT_NO_FOREACH",
            "QT_NO_URL_CAST_FROM_STRING",
            "_USE_MATH_DEFINES"
        ];

        if (project.snapshot)
            defs.push("TILED_SNAPSHOT");

        if (project.windowsLayout)
            defs.push("TILED_WINDOWS_LAYOUT");

        if (qbs.targetOS.contains("linux") && project.dbus && Qt.dbus.present)
            defs.push("TILED_ENABLE_DBUS");

        if (project.sentry)
            defs.push("TILED_SENTRY");

        return defs;
    }

    Properties {
        condition: project.sentry
        cpp.dynamicLibraries: outer.concat(["sentry"])
        cpp.libraryPaths: outer.concat(["../../sentry-native/install/lib"])
    }

    Group {
        name: "Precompiled header"
        files: ["pch.h"]
        fileTags: ["cpp_pch_src"]
    }

    files: [
        "aboutdialog.cpp",
        "aboutdialog.h",
        "aboutdialog.ui",
        "abstractobjecttool.cpp",
        "abstractobjecttool.h",
        "abstracttilefilltool.cpp",
        "abstracttilefilltool.h",
        "abstracttileselectiontool.cpp",
        "abstracttileselectiontool.h",
        "abstracttiletool.cpp",
        "abstracttiletool.h",
        "abstracttool.cpp",
        "abstracttool.h",
        "abstractworldtool.cpp",
        "abstractworldtool.h",
        "actionmanager.cpp",
        "actionmanager.h",
        "actionsearch.cpp",
        "actionsearch.h",
        "addremovelayer.cpp",
        "addremovelayer.h",
        "addremovemapobject.cpp",
        "addremovemapobject.h",
        "addremovetiles.cpp",
        "addremovetileset.cpp",
        "addremovetileset.h",
        "addremovetiles.h",
        "addremovewangset.cpp",
        "addremovewangset.h",
        "adjusttileindexes.cpp",
        "adjusttileindexes.h",
        "automapper.cpp",
        "automapper.h",
        "automapperwrapper.cpp",
        "automapperwrapper.h",
        "automappingmanager.cpp",
        "automappingmanager.h",
        "automappingutils.cpp",
        "automappingutils.h",
        "brokenlinks.cpp",
        "brokenlinks.h",
        "brushitem.cpp",
        "brushitem.h",
        "bucketfilltool.cpp",
        "bucketfilltool.h",
        "capturestamphelper.cpp",
        "capturestamphelper.h",
        "changeevents.h",
        "changeimagelayerproperty.cpp",
        "changeimagelayerproperty.h",
        "changelayer.cpp",
        "changelayer.h",
        "changemapobject.cpp",
        "changemapobject.h",
        "changemapobjectsorder.cpp",
        "changemapobjectsorder.h",
        "changemapproperty.h",
        "changeobjectgroupproperties.cpp",
        "changeobjectgroupproperties.h",
        "changepolygon.cpp",
        "changepolygon.h",
        "changeproperties.cpp",
        "changeproperties.h",
        "changeselectedarea.cpp",
        "changeselectedarea.h",
        "changetile.cpp",
        "changetile.h",
        "changetileanimation.cpp",
        "changetileanimation.h",
        "changetileimagesource.cpp",
        "changetileimagesource.h",
        "changetileobjectgroup.cpp",
        "changetileobjectgroup.h",
        "changetilewangid.cpp",
        "changetilewangid.h",
        "changevalue.h",
        "changewangcolordata.cpp",
        "changewangcolordata.h",
        "changewangsetdata.cpp",
        "changewangsetdata.h",
        "changeworld.cpp",
        "changeworld.h",
        "clipboardmanager.cpp",
        "clipboardmanager.h",
        "colorbutton.cpp",
        "colorbutton.h",
        "commandbutton.cpp",
        "commandbutton.h",
        "command.cpp",
        "commanddatamodel.cpp",
        "commanddatamodel.h",
        "commanddialog.cpp",
        "commanddialog.h",
        "commanddialog.ui",
        "command.h",
        "commandmanager.cpp",
        "commandmanager.h",
        "commandsedit.cpp",
        "commandsedit.h",
        "commandsedit.ui",
        "consoledock.cpp",
        "consoledock.h",
        "createcapsuleobjecttool.cpp",
        "createcapsuleobjecttool.h",
        "createellipseobjecttool.cpp",
        "createellipseobjecttool.h",
        "createobjecttool.cpp",
        "createobjecttool.h",
        "createpointobjecttool.cpp",
        "createpointobjecttool.h",
        "createpolygonobjecttool.cpp",
        "createpolygonobjecttool.h",
        "createrectangleobjecttool.cpp",
        "createrectangleobjecttool.h",
        "createscalableobjecttool.cpp",
        "createscalableobjecttool.h",
        "createtemplatetool.cpp",
        "createtemplatetool.h",
        "createtextobjecttool.cpp",
        "createtextobjecttool.h",
        "createtileobjecttool.cpp",
        "createtileobjecttool.h",
        "debugdrawitem.cpp",
        "debugdrawitem.h",
        "document.cpp",
        "document.h",
        "documentmanager.cpp",
        "documentmanager.h",
        "donationpopup.cpp",
        "donationpopup.h",
        "editableasset.cpp",
        "editableasset.h",
        "editablegrouplayer.cpp",
        "editablegrouplayer.h",
        "editableimagelayer.cpp",
        "editableimagelayer.h",
        "editablelayer.cpp",
        "editablelayer.h",
        "editablemap.cpp",
        "editablemap.h",
        "editablemapobject.cpp",
        "editablemapobject.h",
        "editableobject.cpp",
        "editableobject.h",
        "editableobjectgroup.cpp",
        "editableobjectgroup.h",
        "editableproject.cpp",
        "editableproject.h",
        "editableselectedarea.cpp",
        "editableselectedarea.h",
        "editabletile.cpp",
        "editabletile.h",
        "editabletilelayer.cpp",
        "editabletilelayer.h",
        "editabletileset.cpp",
        "editabletileset.h",
        "editablewangset.cpp",
        "editablewangset.h",
        "editableworld.cpp",
        "editableworld.h",
        "editor.cpp",
        "editor.h",
        "editpolygontool.cpp",
        "editpolygontool.h",
        "eraser.cpp",
        "eraser.h",
        "erasetiles.cpp",
        "erasetiles.h",
        "exportasimagedialog.cpp",
        "exportasimagedialog.h",
        "exportasimagedialog.ui",
        "exporthelper.cpp",
        "exporthelper.h",
        "expressionspinbox.cpp",
        "expressionspinbox.h",
        "filechangedwarning.cpp",
        "filechangedwarning.h",
        "fileedit.cpp",
        "fileedit.h",
        "filteredit.cpp",
        "filteredit.h",
        "flexiblescrollbar.cpp",
        "flexiblescrollbar.h",
        "flipmapobjects.cpp",
        "flipmapobjects.h",
        "geometry.cpp",
        "geometry.h",
        "grouplayeritem.cpp",
        "grouplayeritem.h",
        "iconcheckdelegate.cpp",
        "iconcheckdelegate.h",
        "id.cpp",
        "id.h",
        "imagecolorpickerwidget.cpp",
        "imagecolorpickerwidget.h",
        "imagecolorpickerwidget.ui",
        "imagelayeritem.cpp",
        "imagelayeritem.h",
        "clickablelabel.cpp",
        "clickablelabel.h",
        "issuescounter.cpp",
        "issuescounter.h",
        "issuesdock.cpp",
        "issuesdock.h",
        "issuesmodel.cpp",
        "issuesmodel.h",
        "languagemanager.cpp",
        "languagemanager.h",
        "layerdock.cpp",
        "layerdock.h",
        "layeritem.cpp",
        "layeritem.h",
        "layermodel.cpp",
        "layermodel.h",
        "layeroffsettool.cpp",
        "layeroffsettool.h",
        "listedit.cpp",
        "listedit.h",
        "locatorwidget.cpp",
        "locatorwidget.h",
        "magicwandtool.h",
        "magicwandtool.cpp",
        "maintoolbar.cpp",
        "maintoolbar.h",
        "mainwindow.cpp",
        "mainwindow.h",
        "mainwindow.ui",
        "mapdocumentactionhandler.cpp",
        "mapdocumentactionhandler.h",
        "mapdocument.cpp",
        "mapdocument.h",
        "mapeditor.cpp",
        "mapeditor.h",
        "mapitem.cpp",
        "mapitem.h",
        "mapobjectitem.cpp",
        "mapobjectitem.h",
        "mapobjectmodel.cpp",
        "mapobjectmodel.h",
        "mapscene.cpp",
        "mapscene.h",
        "mapview.cpp",
        "mapview.h",
        "minimap.cpp",
        "minimapdock.cpp",
        "minimapdock.h",
        "minimap.h",
        "movelayer.cpp",
        "movelayer.h",
        "movemapobjecttogroup.cpp",
        "movemapobjecttogroup.h",
        "newmapdialog.cpp",
        "newmapdialog.h",
        "newmapdialog.ui",
        "newsbutton.cpp",
        "newsbutton.h",
        "newsfeed.cpp",
        "newsfeed.h",
        "newtilesetdialog.cpp",
        "newtilesetdialog.h",
        "newtilesetdialog.ui",
        "newversionbutton.cpp",
        "newversionbutton.h",
        "newversionchecker.cpp",
        "newversionchecker.h",
        "newversiondialog.cpp",
        "newversiondialog.h",
        "newversiondialog.ui",
        "noeditorwidget.cpp",
        "noeditorwidget.h",
        "noeditorwidget.ui",
        "objectgroupitem.cpp",
        "objectgroupitem.h",
        "objectrefdialog.cpp",
        "objectrefdialog.h",
        "objectrefdialog.ui",
        "objectrefedit.cpp",
        "objectrefedit.h",
        "objectreferenceitem.cpp",
        "objectreferenceitem.h",
        "objectreferenceshelper.cpp",
        "objectreferenceshelper.h",
        "objectreferencetool.cpp",
        "objectreferencetool.h",
        "objectsdock.cpp",
        "objectsdock.h",
        "objectselectionitem.cpp",
        "objectselectionitem.h",
        "objectselectiontool.cpp",
        "objectselectiontool.h",
        "objectsview.cpp",
        "objectsview.h",
        "offsetlayer.cpp",
        "offsetlayer.h",
        "offsetmapdialog.cpp",
        "offsetmapdialog.h",
        "offsetmapdialog.ui",
        "painttilelayer.cpp",
        "painttilelayer.h",
        "pannableviewhelper.cpp",
        "pannableviewhelper.h",
        "pluginlistmodel.cpp",
        "pluginlistmodel.h",
        "pointhandle.cpp",
        "pointhandle.h",
        "popupwidget.cpp",
        "popupwidget.h",
        "preferences.cpp",
        "preferencesdialog.cpp",
        "preferencesdialog.h",
        "preferencesdialog.ui",
        "preferences.h",
        "project.cpp",
        "project.h",
        "projectdock.cpp",
        "projectdock.h",
        "projectdocument.cpp",
        "projectdocument.h",
        "projectmanager.cpp",
        "projectmanager.h",
        "projectmodel.cpp",
        "projectmodel.h",
        "projectpropertiesdialog.cpp",
        "projectpropertiesdialog.h",
        "projectpropertiesdialog.ui",
        "propertiesdock.cpp",
        "propertiesdock.h",
        "propertiesview.cpp",
        "propertiesview.h",
        "propertieswidget.cpp",
        "propertieswidget.h",
        "propertyeditorwidgets.cpp",
        "propertyeditorwidgets.h",
        "propertytypeseditor.cpp",
        "propertytypeseditor.h",
        "propertytypeseditor.ui",
        "propertytypesmodel.cpp",
        "propertytypesmodel.h",
        "qmldock.cpp",
        "qmldock.h",
        "qmlextension.cpp",
        "qmlextension.h",
        "raiselowerhelper.cpp",
        "raiselowerhelper.h",
        "randompicker.h",
        "rangeset.h",
        "regionvaluetype.cpp",
        "regionvaluetype.h",
        "relocatetiles.cpp",
        "relocatetiles.h",
        "reparentlayers.cpp",
        "reparentlayers.h",
        "replacetemplate.cpp",
        "replacetemplate.h",
        "replacetileset.cpp",
        "replacetileset.h",
        "resizedialog.cpp",
        "resizedialog.h",
        "resizedialog.ui",
        "resizehelper.cpp",
        "resizehelper.h",
        "resizemap.cpp",
        "resizemap.h",
        "resizetilelayer.cpp",
        "resizetilelayer.h",
        "reversingproxymodel.cpp",
        "reversingproxymodel.h",
        "scriptbase64.cpp",
        "scriptbase64.h",
        "scriptdialog.cpp",
        "scriptdialog.h",
        "scriptedaction.cpp",
        "scriptedaction.h",
        "scriptedfileformat.cpp",
        "scriptedfileformat.h",
        "scriptedtool.cpp",
        "scriptedtool.h",
        "scriptfile.cpp",
        "scriptfile.h",
        "scriptfileformatwrappers.cpp",
        "scriptfileformatwrappers.h",
        "scriptfileinfo.cpp",
        "scriptfileinfo.h",
        "scriptgeometry.cpp",
        "scriptgeometry.h",
        "scriptimage.cpp",
        "scriptimage.h",
        "scriptmanager.cpp",
        "scriptmanager.h",
        "scriptmodule.cpp",
        "scriptmodule.h",
        "scriptprocess.cpp",
        "scriptprocess.h",
        "scriptsession.cpp",
        "scriptsession.h",
        "scriptpropertytype.cpp",
        "scriptpropertytype.h",
        "selectionrectangle.cpp",
        "selectionrectangle.h",
        "selectsametiletool.cpp",
        "selectsametiletool.h",
        "session.cpp",
        "session.h",
        "shapefilltool.cpp",
        "shapefilltool.h",
        "shortcutsettingspage.cpp",
        "shortcutsettingspage.h",
        "shortcutsettingspage.ui",
        "snaphelper.cpp",
        "snaphelper.h",
        "stampactions.cpp",
        "stampactions.h",
        "stampbrush.cpp",
        "stampbrush.h",
        "stylehelper.cpp",
        "stylehelper.h",
        "swaptiles.cpp",
        "swaptiles.h",
        "tabbar.cpp",
        "tabbar.h",
        "templatesdock.cpp",
        "templatesdock.h",
        "texteditordialog.cpp",
        "texteditordialog.h",
        "texteditordialog.ui",
        "textpropertyedit.cpp",
        "textpropertyedit.h",
        "tileanimationeditor.cpp",
        "tileanimationeditor.h",
        "tileanimationeditor.ui",
        "tilecollisiondock.cpp",
        "tilecollisiondock.h",
        "tiledapplication.cpp",
        "tiledapplication.h",
        "tilededitor_global.h",
        "tiledproxystyle.cpp",
        "tiledproxystyle.h",
        "tilelayeredit.cpp",
        "tilelayeredit.h",
        "tilelayeritem.cpp",
        "tilelayeritem.h",
        "tilelayerwangedit.cpp",
        "tilelayerwangedit.h",
        "tilepainter.cpp",
        "tilepainter.h",
        "tileselectionitem.cpp",
        "tileselectionitem.h",
        "tilehighlightitem.cpp",
        "tilehighlightitem.h",
        "tileselectiontool.cpp",
        "tileselectiontool.h",
        "tilesetchanges.cpp",
        "tilesetchanges.h",
        "tilesetdock.cpp",
        "tilesetdock.h",
        "tilesetdocument.cpp",
        "tilesetdocument.h",
        "tilesetdocumentsmodel.cpp",
        "tilesetdocumentsmodel.h",
        "tileseteditor.cpp",
        "tileseteditor.h",
        "tilesetmodel.cpp",
        "tilesetmodel.h",
        "tilesetparametersedit.cpp",
        "tilesetparametersedit.h",
        "tilesetview.cpp",
        "tilesetview.h",
        "tilesetwangsetmodel.cpp",
        "tilesetwangsetmodel.h",
        "tilestamp.cpp",
        "tilestamp.h",
        "tilestampmanager.cpp",
        "tilestampmanager.h",
        "tilestampmodel.cpp",
        "tilestampmodel.h",
        "tilestampsdock.cpp",
        "tilestampsdock.h",
        "toolmanager.cpp",
        "toolmanager.h",
        "transformmapobjects.cpp",
        "transformmapobjects.h",
        "treeviewcombobox.cpp",
        "treeviewcombobox.h",
        "undocommands.cpp",
        "undocommands.h",
        "undodock.cpp",
        "undodock.h",
        "utils.cpp",
        "utils.h",
        "variantmapproperty.cpp",
        "variantmapproperty.h",
        "wangbrush.cpp",
        "wangbrush.h",
        "wangcolormodel.cpp",
        "wangcolormodel.h",
        "wangcolorview.cpp",
        "wangcolorview.h",
        "wangdock.cpp",
        "wangdock.h",
        "wangfiller.cpp",
        "wangfiller.h",
        "wangoverlay.cpp",
        "wangoverlay.h",
        "wangsetmodel.cpp",
        "wangsetmodel.h",
        "wangsetview.cpp",
        "wangsetview.h",
        "wangtemplatemodel.cpp",
        "wangtemplatemodel.h",
        "wangtemplateview.cpp",
        "wangtemplateview.h",
        "worlddocument.cpp",
        "worlddocument.h",
        "worldmanager.cpp",
        "worldmanager.h",
        "worldmovemaptool.cpp",
        "worldmovemaptool.h",
        "worldpropertiesdialog.cpp",
        "worldpropertiesdialog.h",
        "worldpropertiesdialog.ui",
        "zoomable.cpp",
        "zoomable.h",
    ]

    Group {
        name: "Resources"
        Qt.core.resourceSourceBase: "resources/"
        files: "resources/**"
        fileTags: ["qt.core.resource_data"]
    }

    Group {
        name: "Sentry"
        condition: project.sentry
        files: [
            "sentryhelper.cpp",
            "sentryhelper.h",
        ]
    }

    Properties {
        condition: qbs.targetOS.contains("macos")
        bundle.isBundle: false
        cpp.sonamePrefix: "@rpath"
    }

    Export {
        Depends { name: "cpp" }
        Depends { name: "libtiled" }
        Depends { name: "qtsingleapplication" }
        Depends { name: "Qt"; submodules: ["qml"] }
        cpp.includePaths: exportingProduct.sourceDirectory
    }

    install: !qbs.targetOS.contains("darwin")
    installDir: {
        if (qbs.targetOS.contains("windows"))
            if (project.windowsLayout)
                return ""
            else
                return "bin"
        else
            return project.libDir
    }
}
