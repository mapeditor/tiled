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

    Group {
        name: "qmldir"
        qbs.install: true
        qbs.installDir: "qml/org/mapeditor/Tiled"
        files: [
            "qmldir",
        ]
    }

    Group {
        qbs.install: true
        qbs.installDir: "qml/org/mapeditor/Tiled"
        fileTagsFilter: "dynamiclibrary"
    }
}
