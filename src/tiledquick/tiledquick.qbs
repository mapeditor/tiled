import qbs 1.0
import qbs.File

QtGuiApplication {
    name: "tiledquick"
    targetName: name
    builtByDefault: false
    condition: Qt.core.versionMinor > 10

    Depends {
        name: "Qt"
        submodules: ["core", "quick", "widgets"]
        versionAtLeast: "5.10"
    }
    Depends {
        name: "tiledquickplugin"
        cpp.link: false
    }

    cpp.includePaths: ["."]
    cpp.rpaths: qbs.targetOS.contains("darwin") ? ["@loader_path/../Frameworks"] : ["$ORIGIN/../lib"]
    cpp.cxxLanguageVersion: "c++14"

    files: [
        "fonts/fonts.qrc",
        "main.cpp",
        "qml/qml.qrc",
        "qml/qtquickcontrols2.conf",
    ]

    Properties {
        condition: qbs.targetOS.contains("darwin")
        targetName: "Tiled Quick"
    }

    Group {
        condition: !qbs.targetOS.contains("darwin")
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return ""
            else
                return "bin"
        }
        fileTagsFilter: product.type
    }

    // This is necessary to install the app bundle (OS X)
    Group {
        fileTagsFilter: ["bundle.content"]
        qbs.install: true
        qbs.installDir: "."
        qbs.installSourceBase: product.buildDirectory
    }

    // Include libtiled.dylib in the app bundle
    Depends {
        condition: qbs.targetOS.contains("darwin")
        name: "libtiled"
        cpp.link: false
    }
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
}
