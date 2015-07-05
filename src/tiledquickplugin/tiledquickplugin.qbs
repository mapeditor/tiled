import qbs 1.0

DynamicLibrary {
    targetName: "tiledquickplugin"

    condition: Qt.core.versionMajor >= 5

    Depends { name: "libtiled" }
    Depends { name: "Qt.core" }     // for the Qt version check
    Depends {
        condition: Qt.core.versionMajor >= 5
        name: "Qt"; submodules: ["qml", "quick"]
    }

    files: [
        "mapitem.cpp",
        "mapitem.h",
        "maploader.cpp",
        "maploader.h",
        "tiledquickplugin.cpp",
        "tiledquickplugin.h",
        "tilelayeritem.cpp",
        "tilelayeritem.h",
        "tilesnode.cpp",
        "tilesnode.h",
    ]

    property string installBase: qbs.targetOS.contains("osx") ? "Tiled Quick.app/Contents/" : ""

    Group {
        name: "qmldir"
        files: "qmldir"
        fileTags: "qmldir"
    }

    Group {
        qbs.install: true
        qbs.installDir: installBase + "qml/org/mapeditor/Tiled"
        fileTagsFilter: ["dynamiclibrary", "qmldir"]
    }
}
