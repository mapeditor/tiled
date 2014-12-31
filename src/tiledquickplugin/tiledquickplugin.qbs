import qbs 1.0

DynamicLibrary {
    targetName: "tiledquickplugin"

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["qml", "quick"] }

    files: [
        "tiledquickplugin.cpp",
        "tiledquickplugin.h",
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
