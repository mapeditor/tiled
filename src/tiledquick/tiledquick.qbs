import qbs.Environment
import qbs.File
import qbs.Utilities

QtGuiApplication {
    name: "tiledquick"
    targetName: name
    builtByDefault: Environment.getEnv("BUILD_TILEDQUICK") == "true"
    condition: Utilities.versionCompare(Qt.core.version, "6.5") >= 0

    readonly property bool qtcRunnable: builtByDefault

    Depends {
        name: "Qt"
        submodules: ["core", "quick", "widgets"]
        versionAtLeast: "6.5"
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
        if (qbs.toolchain.contains("msvc"))
            flags.push("/permissive-");
        return flags;
    }
    cpp.defines: [
        "QT_DISABLE_DEPRECATED_BEFORE=QT_VERSION_CHECK(6,5,0)",
        "QT_NO_DEPRECATED_WARNINGS",
        "QT_NO_FOREACH",
        "QT_NO_URL_CAST_FROM_STRING"
    ]

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
