import qbs 1.0

DynamicLibrary {
    targetName: "tiledquickplugin"

    Depends { name: "libtiled" }
    Depends {
        name: "Qt"; submodules: ["qml", "quick"]
        versionAtLeast: "5.4"
    }

    cpp.cxxLanguageVersion: "c++11"

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
