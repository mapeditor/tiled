import qbs.File
import qbs.FileInfo
import qbs.TextFile

TiledQtGuiApplication {
    name: "tiled"
    targetName: name
    version: project.version
    consoleApplication: false

    Depends { name: "libtilededitor" }
    Depends { name: "ib"; condition: qbs.targetOS.contains("macos") }
    Depends { name: "Qt.gui-private"; condition: qbs.targetOS.contains("windows") && Qt.core.versionMajor >= 6 }
    Depends { name: "texttemplate"; condition: qbs.targetOS.contains("windows") }

    property bool qtcRunnable: true

    cpp.includePaths: {
        var paths = ["."];

        if (project.sentry)
            paths.push("../../sentry-native/install/include");

        return paths;
    }

    cpp.defines: {
        var defs = base;

        if (project.sentry)
            defs.push("TILED_SENTRY");

        return defs;
    }

    files: [
        "commandlineparser.cpp",
        "commandlineparser.h",
        "main.cpp",
    ]

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.frameworks: ["Foundation"]
        bundle.identifierPrefix: "org.mapeditor"
        ib.appIconName: "tiled-icon-mac"
        targetName: "Tiled"
    }

    Properties {
        condition: qbs.targetOS.contains("windows")

        texttemplate.outputTag: "rc"
        texttemplate.dict: {
            var versionArray = project.version.split(".");
            if (versionArray.length == 3)
                versionArray.push("0");

            return {
                version: project.version,
                version_csv: versionArray.join(",")
            };
        }
    }

    Group {
        condition: qbs.targetOS.contains("macos")
        name: "Public DSA Key File"
        files: ["../../dist/dsa_pub.pem"]
        qbs.install: true
        qbs.installDir: "Tiled.app/Contents/Resources"
    }

    Group {
        name: "macOS (Info.plist and icons)"
        condition: qbs.targetOS.contains("macos")
        files: [
            "Info.plist",
            "images/tiled.xcassets",
        ]
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

    // Include libtiled.dylib and libtilededitor.dylib in the app bundle
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
        condition: qbs.targetOS.contains("windows")
        files: [ "tiled.rc.in" ]
    }
}
