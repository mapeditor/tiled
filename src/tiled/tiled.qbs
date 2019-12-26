import qbs 1.0
import qbs.File
import qbs.FileInfo
import qbs.TextFile

QtGuiApplication {
    name: "tiled"
    targetName: name
    version: project.version

    Depends { name: "libtiled" }
    Depends { name: "translations" }
    Depends { name: "qtpropertybrowser" }
    Depends { name: "qtsingleapplication" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt"; submodules: ["core", "widgets", "qml"]; versionAtLeast: "5.6" }

    property bool qtcRunnable: true

    cpp.includePaths: [
                ".",
                "../../zstd/lib"
            ]

    cpp.useRPaths: project.useRPaths
    cpp.rpaths: {
        if (qbs.targetOS.contains("darwin"))
            return ["@loader_path/../Frameworks"];
        else if (project.linuxArchive)
            return ["$ORIGIN/lib"];
        else
            return ["$ORIGIN/../lib"];
    }
    cpp.useCxxPrecompiledHeader: qbs.buildVariant != "debug"
    cpp.cxxLanguageVersion: "c++14"

    cpp.defines: {
        var defs = [
            "TILED_VERSION=" + version,
            "QT_DEPRECATED_WARNINGS",
            "QT_DISABLE_DEPRECATED_BEFORE=0x050900",
            "QT_NO_CAST_FROM_ASCII",
            "QT_NO_CAST_TO_ASCII",
            "QT_NO_FOREACH",
            "QT_NO_URL_CAST_FROM_STRING",
            "_USE_MATH_DEFINES"
        ];
        if (project.snapshot)
            defs.push("TILED_SNAPSHOT");

        if (project.enableZstd)
            defs.push("TILED_ZSTD_SUPPORT");

        return defs;
    }

    consoleApplication: false

    Group {
        name: "Precompiled header"
        files: ["pch.h"]
        fileTags: ["cpp_pch_src"]
    }

    files: [
        "dialogs/aboutdialog.cpp",
        "dialogs/aboutdialog.h",
        "dialogs/aboutdialog.ui",
        "tools/abstractobjecttool.cpp",
        "tools/abstractobjecttool.h",
        "tools/abstracttilefilltool.cpp",
        "tools/abstracttilefilltool.h",
        "tools/abstracttileselectiontool.cpp",
        "tools/abstracttileselectiontool.h",
        "tools/abstracttiletool.cpp",
        "tools/abstracttiletool.h",
        "tools/abstracttool.cpp",
        "tools/abstracttool.h",
        "actionmanager.cpp",
        "actionmanager.h",
        "dialogs/addpropertydialog.cpp",
        "dialogs/addpropertydialog.h",
        "dialogs/addpropertydialog.ui",
        "addremovelayer.cpp",
        "addremovelayer.h",
        "addremovemapobject.cpp",
        "addremovemapobject.h",
        "addremoveterrain.cpp",
        "addremoveterrain.h",
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
        "tools/brushitem.cpp",
        "tools/brushitem.h",
        "tools/bucketfilltool.cpp",
        "tools/bucketfilltool.h",
        "capturestamphelper.cpp",
        "capturestamphelper.h",
        "changeevents.h",
        "changeimagelayerproperties.cpp",
        "changeimagelayerproperties.h",
        "changelayer.cpp",
        "changelayer.h",
        "changemapobject.cpp",
        "changemapobject.h",
        "changemapobjectsorder.cpp",
        "changemapobjectsorder.h",
        "changemapproperty.cpp",
        "changemapproperty.h",
        "changeobjectgroupproperties.cpp",
        "changeobjectgroupproperties.h",
        "changepolygon.cpp",
        "changepolygon.h",
        "changeproperties.cpp",
        "changeproperties.h",
        "changeselectedarea.cpp",
        "changeselectedarea.h",
        "changeterrain.cpp",
        "changeterrain.h",
        "changetile.cpp",
        "changetile.h",
        "changetileanimation.cpp",
        "changetileanimation.h",
        "changetileimagesource.cpp",
        "changetileimagesource.h",
        "changetileobjectgroup.cpp",
        "changetileobjectgroup.h",
        "changetileprobability.cpp",
        "changetileprobability.h",
        "changetileterrain.cpp",
        "changetileterrain.h",
        "changetilewangid.cpp",
        "changetilewangid.h",
        "changewangcolordata.cpp",
        "changewangcolordata.h",
        "changewangsetdata.cpp",
        "changewangsetdata.h",
        "clipboardmanager.cpp",
        "clipboardmanager.h",
        "colorbutton.cpp",
        "colorbutton.h",
        "commandbutton.cpp",
        "commandbutton.h",
        "command.cpp",
        "commanddatamodel.cpp",
        "commanddatamodel.h",
        "dialogs/commanddialog.cpp",
        "dialogs/commanddialog.h",
        "dialogs/commanddialog.ui",
        "command.h",
        "commandlineparser.cpp",
        "commandlineparser.h",
        "commandmanager.cpp",
        "commandmanager.h",
        "docks/consoledock.cpp",
        "docks/consoledock.h",
        "tools/createellipseobjecttool.cpp",
        "tools/createellipseobjecttool.h",
        "tools/createobjecttool.cpp",
        "tools/createobjecttool.h",
        "tools/createpointobjecttool.cpp",
        "tools/createpointobjecttool.h",
        "tools/createpolygonobjecttool.cpp",
        "tools/createpolygonobjecttool.h",
        "tools/createrectangleobjecttool.cpp",
        "tools/createrectangleobjecttool.h",
        "tools/createscalableobjecttool.cpp",
        "tools/createscalableobjecttool.h",
        "tools/createtemplatetool.cpp",
        "tools/createtemplatetool.h",
        "tools/createtextobjecttool.cpp",
        "tools/createtextobjecttool.h",
        "tools/createtileobjecttool.cpp",
        "tools/createtileobjecttool.h",
        "document.cpp",
        "document.h",
        "documentmanager.cpp",
        "documentmanager.h",
        "dialogs/donationdialog.cpp",
        "dialogs/donationdialog.h",
        "dialogs/donationdialog.ui",
        "scripting/editableasset.cpp",
        "scripting/editableasset.h",
        "scripting/editablegrouplayer.cpp",
        "scripting/editablegrouplayer.h",
        "scripting/editableimagelayer.cpp",
        "scripting/editableimagelayer.h",
        "scripting/editablelayer.cpp",
        "scripting/editablelayer.h",
        "scripting/editablemanager.cpp",
        "scripting/editablemanager.h",
        "scripting/editablemap.cpp",
        "scripting/editablemap.h",
        "scripting/editablemapobject.cpp",
        "scripting/editablemapobject.h",
        "scripting/editableobject.cpp",
        "scripting/editableobject.h",
        "scripting/editableobjectgroup.cpp",
        "scripting/editableobjectgroup.h",
        "scripting/editableselectedarea.cpp",
        "scripting/editableselectedarea.h",
        "scripting/editableterrain.cpp",
        "scripting/editableterrain.h",
        "scripting/editabletile.cpp",
        "scripting/editabletile.h",
        "scripting/editabletilelayer.cpp",
        "scripting/editabletilelayer.h",
        "scripting/editabletileset.cpp",
        "scripting/editabletileset.h",
        "editor.cpp",
        "editor.h",
        "tools/editpolygontool.cpp",
        "tools/editpolygontool.h",
        "tools/eraser.cpp",
        "tools/eraser.h",
        "tools/erasetiles.cpp",
        "tools/erasetiles.h",
        "dialogs/exportasimagedialog.cpp",
        "dialogs/exportasimagedialog.h",
        "dialogs/exportasimagedialog.ui",
        "exporthelper.cpp",
        "exporthelper.h",
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
        "docks/issuesdock.cpp",
        "docks/issuesdock.h",
        "issuesmodel.cpp",
        "issuesmodel.h",
        "languagemanager.cpp",
        "languagemanager.h",
        "docks/layerdock.cpp",
        "docks/layerdock.h",
        "layeritem.cpp",
        "layeritem.h",
        "layermodel.cpp",
        "layermodel.h",
        "tools/layeroffsettool.cpp",
        "tools/layeroffsettool.h",
        "tools/magicwandtool.h",
        "tools/magicwandtool.cpp",
        "main.cpp",
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
        "docks/minimapdock.cpp",
        "docks/minimapdock.h",
        "minimap.h",
        "minimaprenderer.cpp",
        "minimaprenderer.h",
        "movelayer.cpp",
        "movelayer.h",
        "movemapobject.cpp",
        "movemapobject.h",
        "movemapobjecttogroup.cpp",
        "movemapobjecttogroup.h",
        "moveterrain.cpp",
        "moveterrain.h",
        "dialogs/newmapdialog.cpp",
        "dialogs/newmapdialog.h",
        "dialogs/newmapdialog.ui",
        "newsbutton.cpp",
        "newsbutton.h",
        "newsfeed.cpp",
        "newsfeed.h",
        "dialogs/newtilesetdialog.cpp",
        "dialogs/newtilesetdialog.h",
        "dialogs/newtilesetdialog.ui",
        "newversionbutton.cpp",
        "newversionbutton.h",
        "newversionchecker.cpp",
        "newversionchecker.h",
        "dialogs/newversiondialog.cpp",
        "dialogs/newversiondialog.h",
        "dialogs/newversiondialog.ui",
        "noeditorwidget.cpp",
        "noeditorwidget.h",
        "noeditorwidget.ui",
        "objectgroupitem.cpp",
        "objectgroupitem.h",
        "docks/objectsdock.cpp",
        "docks/objectsdock.h",
        "objectselectionitem.cpp",
        "objectselectionitem.h",
        "tools/objectselectiontool.cpp",
        "tools/objectselectiontool.h",
        "objectsview.cpp",
        "objectsview.h",
        "objecttemplatemodel.cpp",
        "objecttemplatemodel.h",
        "objecttypeseditor.cpp",
        "objecttypeseditor.h",
        "objecttypeseditor.ui",
        "objecttypesmodel.cpp",
        "objecttypesmodel.h",
        "offsetlayer.cpp",
        "offsetlayer.h",
        "dialogs/offsetmapdialog.cpp",
        "dialogs/offsetmapdialog.h",
        "dialogs/offsetmapdialog.ui",
        "painttilelayer.cpp",
        "painttilelayer.h",
        "pluginlistmodel.cpp",
        "pluginlistmodel.h",
        "pointhandle.cpp",
        "pointhandle.h",
        "preferences.cpp",
        "dialogs/preferencesdialog.cpp",
        "dialogs/preferencesdialog.h",
        "dialogs/preferencesdialog.ui",
        "preferences.h",
        "project.cpp",
        "project.h",
        "docks/projectdock.cpp",
        "docks/projectdock.h",
        "projectmodel.cpp",
        "projectmodel.h",
        "docks/propertiesdock.cpp",
        "docks/propertiesdock.h",
        "propertybrowser.cpp",
        "propertybrowser.h",
        "raiselowerhelper.cpp",
        "raiselowerhelper.h",
        "randompicker.h",
        "rangeset.h",
        "regionvaluetype.cpp",
        "regionvaluetype.h",
        "renamewangset.cpp",
        "renamewangset.h",
        "reparentlayers.cpp",
        "reparentlayers.h",
        "replacetemplate.cpp",
        "replacetemplate.h",
        "replacetileset.cpp",
        "replacetileset.h",
        "dialogs/resizedialog.cpp",
        "dialogs/resizedialog.h",
        "dialogs/resizedialog.ui",
        "resizehelper.cpp",
        "resizehelper.h",
        "resizemap.cpp",
        "resizemap.h",
        "resizemapobject.cpp",
        "resizemapobject.h",
        "resizetilelayer.cpp",
        "resizetilelayer.h",
        "reversingproxymodel.cpp",
        "reversingproxymodel.h",
        "rotatemapobject.cpp",
        "rotatemapobject.h",
        "scripting/scriptedaction.cpp",
        "scripting/scriptedaction.h",
        "scripting/scriptedfileformat.cpp",
        "scripting/scriptedfileformat.h",
        "scripting/scriptedtool.cpp",
        "scripting/scriptedtool.h",
        "scripting/scriptfile.cpp",
        "scripting/scriptfile.h",
        "scripting/scriptmanager.cpp",
        "scripting/scriptmanager.h",
        "scripting/scriptmodule.cpp",
        "scripting/scriptmodule.h",
        "selectionrectangle.cpp",
        "selectionrectangle.h",
        "tools/selectsametiletool.cpp",
        "tools/selectsametiletool.h",
        "session.cpp",
        "session.h",
        "tools/shapefilltool.cpp",
        "tools/shapefilltool.h",
        "shortcutsettingspage.cpp",
        "shortcutsettingspage.h",
        "shortcutsettingspage.ui",
        "snaphelper.cpp",
        "snaphelper.h",
        "stampactions.cpp",
        "stampactions.h",
        "tools/stampbrush.cpp",
        "tools/stampbrush.h",
        "stylehelper.cpp",
        "stylehelper.h",
        "swaptiles.cpp",
        "swaptiles.h",
        "docks/templatesdock.cpp",
        "docks/templatesdock.h",
        "tools/terrainbrush.cpp",
        "tools/terrainbrush.h",
        "docks/terraindock.cpp",
        "docks/terraindock.h",
        "terrainmodel.cpp",
        "terrainmodel.h",
        "terrainview.cpp",
        "terrainview.h",
        "dialogs/texteditordialog.cpp",
        "dialogs/texteditordialog.h",
        "dialogs/texteditordialog.ui",
        "textpropertyedit.cpp",
        "textpropertyedit.h",
        "tileanimationeditor.cpp",
        "tileanimationeditor.h",
        "tileanimationeditor.ui",
        "docks/tilecollisiondock.cpp",
        "docks/tilecollisiondock.h",
        "tiledapplication.cpp",
        "tiledapplication.h",
        "tiled.qrc",
        "tiledproxystyle.cpp",
        "tiledproxystyle.h",
        "tilelayeredit.cpp",
        "tilelayeredit.h",
        "tilelayeritem.cpp",
        "tilelayeritem.h",
        "tilepainter.cpp",
        "tilepainter.h",
        "tileselectionitem.cpp",
        "tileselectionitem.h",
        "tools/tileselectiontool.cpp",
        "tools/tileselectiontool.h",
        "tilesetchanges.cpp",
        "tilesetchanges.h",
        "docks/tilesetdock.cpp",
        "docks/tilesetdock.h",
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
        "tilesetterrainmodel.cpp",
        "tilesetterrainmodel.h",
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
        "docks/tilestampsdock.cpp",
        "docks/tilestampsdock.h",
        "tmxmapformat.cpp",
        "tmxmapformat.h",
        "toolmanager.cpp",
        "toolmanager.h",
        "treeviewcombobox.cpp",
        "treeviewcombobox.h",
        "undocommands.cpp",
        "undocommands.h",
        "docks/undodock.cpp",
        "docks/undodock.h",
        "utils.cpp",
        "utils.h",
        "varianteditorfactory.cpp",
        "varianteditorfactory.h",
        "variantpropertymanager.cpp",
        "variantpropertymanager.h",
        "wangbrush.cpp",
        "wangbrush.h",
        "wangcolormodel.cpp",
        "wangcolormodel.h",
        "wangcolorview.cpp",
        "wangcolorview.h",
        "docks/wangdock.cpp",
        "docks/wangdock.h",
        "wangfiller.cpp",
        "wangfiller.h",
        "wangsetmodel.cpp",
        "wangsetmodel.h",
        "wangsetview.cpp",
        "wangsetview.h",
        "wangtemplatemodel.cpp",
        "wangtemplatemodel.h",
        "wangtemplateview.cpp",
        "wangtemplateview.h",
        "zoomable.cpp",
        "zoomable.h",
    ]

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.frameworks: ["Foundation"]
        cpp.cxxFlags: ["-Wno-unknown-pragmas"]
        bundle.identifierPrefix: "org.mapeditor"
        ib.appIconName: "tiled-icon-mac"
        targetName: "Tiled"
    }
    Group {
        name: "macOS"
        condition: qbs.targetOS.contains("macos")
        files: [
            "Info.plist",
            "macsupport.h",
            "macsupport.mm",
        ]
    }

    Group {
        condition: !qbs.targetOS.contains("darwin")
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows")
                    || project.linuxArchive)
                return ""
            else
                return "bin"
        }
        qbs.installSourceBase: product.buildDirectory
        fileTagsFilter: product.type
    }

    Group {
        condition: qbs.targetOS.contains("macos")
        name: "Public DSA Key File"
        files: ["../../dist/dsa_pub.pem"]
        qbs.install: true
        qbs.installDir: "Tiled.app/Contents/Resources"
    }

    Group {
        name: "macOS (icons)"
        condition: qbs.targetOS.contains("macos")
        files: ["images/tiled.xcassets"]
    }

    Group {
        name: "Desktop file (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/applications"
        files: [ "../../org.mapeditor.Tiled.desktop" ]
    }

    Group {
        name: "AppData file (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/metainfo"
        files: [ "../../org.mapeditor.Tiled.appdata.xml" ]
    }

    Group {
        name: "Thumbnailer (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/thumbnailers"
        files: [ "../../mime/tiled.thumbnailer" ]
    }

    Group {
        name: "MIME info (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/mime/packages"
        files: [ "../../mime/org.mapeditor.Tiled.xml" ]
    }

    Group {
        name: "Man page (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/man/man1"
        files: [ "../../man/tiled.1" ]
    }

    Group {
        name: "Icon 16x16 (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/16x16/apps"
        files: [ "images/16/tiled.png" ]
    }

    Group {
        name: "Icon 32x32 (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/32x32/apps"
        files: [ "images/32/tiled.png" ]
    }

    Group {
        name: "Icon scalable (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/scalable/apps"
        files: [ "images/scalable/tiled.svg" ]
    }

    Group {
        name: "MIME icon 16x16 (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/16x16/mimetypes"
        files: [ "images/16/application-x-tiled.png" ]
    }

    Group {
        name: "MIME icon 32x32 (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/32x32/mimetypes"
        files: [ "images/32/application-x-tiled.png" ]
    }

    Group {
        name: "MIME icon scalable (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/icons/hicolor/scalable/mimetypes"
        files: [ "images/scalable/application-x-tiled.svg" ]
    }

    // This is necessary to install the app bundle (OS X)
    Group {
        fileTagsFilter: ["bundle.content"]
        qbs.install: true
        qbs.installDir: "."
        qbs.installSourceBase: product.buildDirectory
    }

    // Include libtiled.dylib in the app bundle
    Rule {
        condition: qbs.targetOS.contains("darwin")
        inputsFromDependencies: "dynamiclibrary"
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "preparing " + input.fileName + " for inclusion in " + product.targetName + ".app";
            cmd.sourceCode = function() { File.copy(input.filePath, output.filePath); };
            return cmd;
        }

        Artifact {
            filePath: input.fileName
            fileTags: "bundle.input"
            bundle._bundleFilePath: product.destinationDirectory + "/" + product.targetName + ".app/Contents/Frameworks/" + input.fileName
        }
    }

    // Generate the tiled.rc file in order to dynamically specify the version
    Group {
        name: "RC file (Windows)"
        files: [ "tiled.rc.in" ]
        fileTags: ["rcIn"]
    }
    Rule {
        inputs: ["rcIn"]
        Artifact {
            filePath: {
                var destdir = FileInfo.joinPaths(product.moduleProperty("Qt.core",
                                                         "generatedFilesDir"), input.fileName);
                return destdir.replace(/\.[^\.]*$/,'')
            }
            fileTags: "rc"
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "prepare " + FileInfo.fileName(output.filePath);
            cmd.highlight = "codegen";

            cmd.sourceCode = function() {
                var i;
                var vars = {};
                var inf = new TextFile(input.filePath);
                var all = inf.readAll();

                var versionArray = project.version.split(".");
                if (versionArray.length == 3)
                    versionArray.push("0");

                // replace vars
                vars['VERSION'] = project.version;
                vars['VERSION_CSV'] = versionArray.join(",");

                for (i in vars) {
                    all = all.replace(new RegExp('@' + i + '@(?!\w)', 'g'), vars[i]);
                }

                var file = new TextFile(output.filePath, TextFile.WriteOnly);
                file.truncate();
                file.write(all);
                file.close();
            }

            return cmd;
        }
    }
}
