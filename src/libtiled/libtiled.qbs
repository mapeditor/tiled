import qbs 1.0

DynamicLibrary {
    targetName: "tiled"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui" }

    Properties {
        condition: !qbs.targetOS.contains("windows")
        cpp.dynamicLibraries: base.concat(["z"])
    }

    cpp.cxxLanguageVersion: "c++11"

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
        "terrain.h",
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
        Depends {
            name: "Qt"
            submodules: ["gui"]
        }

        cpp.includePaths: "."
    }

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return ""
            else
                return "lib"
        }
        fileTagsFilter: "dynamiclibrary"
    }
}
