import qbs.Environment
import qbs.File

QtGuiApplication {
    name: "tiledquick"
    targetName: name
    builtByDefault: Environment.getEnv("BUILD_TILEDQUICK") == "true"

    readonly property bool qtcRunnable: builtByDefault

    Depends {
        name: "Qt"
        submodules: ["core", "quick", "widgets"]
        versionAtLeast: "5.12"
    }
    Depends {
        name: "tiledquickplugin"
        cpp.link: false
    }

    cpp.includePaths: ["."]
    cpp.rpaths: qbs.targetOS.contains("darwin") ? ["@loader_path/../Frameworks"] : ["$ORIGIN/../" + project.libDir]
    cpp.cxxLanguageVersion: "c++17"
    cpp.cxxFlags: {
        var flags = base;
        if (qbs.toolchain.contains("msvc")) {
            if (Qt.core.versionMajor >= 6 && Qt.core.versionMinor >= 3)
                flags.push("/permissive-");
        }
        return flags;
    }

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

    install: true
    installDir: {
        if (project.windowsLayout)
            return ""
        else if (qbs.targetOS.contains("darwin"))
            return "."
        else
            return "bin"
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
