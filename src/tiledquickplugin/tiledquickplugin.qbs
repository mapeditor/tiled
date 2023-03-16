DynamicLibrary {
    targetName: "tiledquickplugin"
    builtByDefault: false

    Depends { name: "libtiled" }
    Depends { name: "libtiledquick" }
    Depends {
        name: "Qt"; submodules: ["qml", "quick"]
        versionAtLeast: "5.12"
    }

    cpp.cxxLanguageVersion: "c++17"
    cpp.cxxFlags: {
        var flags = base;
        if (qbs.toolchain.contains("msvc")) {
            if (Qt.core.versionMajor >= 6 && Qt.core.versionMinor >= 3)
                flags.push("/permissive-");
        }
        return flags;
    }
    cpp.defines: [
        "QT_DISABLE_DEPRECATED_BEFORE=QT_VERSION_CHECK(5,15,0)",
        "QT_NO_DEPRECATED_WARNINGS",
        "QT_NO_CAST_FROM_ASCII",
        "QT_NO_CAST_TO_ASCII",
        "QT_NO_FOREACH",
        "QT_NO_URL_CAST_FROM_STRING"
    ]

    Properties {
        condition: qbs.targetOS.contains("darwin")
        bundle.isBundle: false
    }

    files: [
        "tiledquickplugin.cpp",
        "tiledquickplugin.h"
    ]

    install: true
    installDir: {
        var installBase = qbs.targetOS.contains("darwin") ? "Tiled Quick.app/Contents/" : "";
        return installBase + "qml/org/mapeditor/Tiled";
    }

    Group {
        name: "qmldir"
        files: "qmldir"
        fileTags: "qmldir"

        qbs.install: true
        qbs.installDir: installDir
    }
}
