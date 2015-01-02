import qbs 1.0

DynamicLibrary {
    targetName: "tiledquickplugin"

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["qml", "quick"] }

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
        qbs.install: true
        qbs.installDir: installBase + "qml/org/mapeditor/Tiled"
        files: [
            "qmldir",
        ]
    }

    Group {
        qbs.install: true
        qbs.installDir: installBase + "qml/org/mapeditor/Tiled"
        fileTagsFilter: "dynamiclibrary"
    }
}
