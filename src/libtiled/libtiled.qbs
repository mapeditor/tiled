import qbs 1.0

DynamicLibrary {
    targetName: "tiled"

    destinationDirectory: {
        if (qbs.targetOS.contains("windows"))
            return "bin"
        else
            return "lib"
    }

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui" }

    cpp.dynamicLibraries: ["z"]
    cpp.defines: [
        "TILED_LIBRARY",
        "QT_NO_CAST_FROM_ASCII",
        "QT_NO_CAST_TO_ASCII"
    ]

    files: [
        "compression.cpp",
        "compression.h",
        "gidmapper.cpp",
        "gidmapper.h",
        "hexagonalrenderer.cpp",
        "hexagonalrenderer.h",
        "imagelayer.cpp",
        "imagelayer.h",
        "isometricrenderer.cpp",
        "isometricrenderer.h",
        "layer.cpp",
        "layer.h",
        "map.cpp",
        "map.h",
        "mapobject.cpp",
        "mapobject.h",
        "mapreader.cpp",
        "mapreader.h",
        "mapreaderinterface.h",
        "maprenderer.cpp",
        "maprenderer.h",
        "mapwriter.cpp",
        "mapwriter.h",
        "mapwriterinterface.h",
        "objectgroup.cpp",
        "objectgroup.h",
        "object.h",
        "orthogonalrenderer.cpp",
        "orthogonalrenderer.h",
        "properties.cpp",
        "properties.h",
        "staggeredrenderer.cpp",
        "staggeredrenderer.h",
        "tile.cpp",
        "tiled_global.h",
        "tiled.h",
        "tile.h",
        "tilelayer.cpp",
        "tilelayer.h",
        "tileset.cpp",
        "tileset.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: "."
    }
}
