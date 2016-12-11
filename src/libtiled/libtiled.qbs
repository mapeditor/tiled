import qbs 1.0

DynamicLibrary {
    targetName: "tiled"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui" }

    Properties {
        condition: !(qbs.toolchain.contains("msvc") ||
                     (qbs.toolchain.contains("mingw") && Qt.core.versionMinor < 6))
        cpp.dynamicLibraries: base.concat(["z"])
    }

    cpp.cxxLanguageVersion: "c++11"
    cpp.visibility: "minimal"
    cpp.defines: {
        var defs = [
            "TILED_LIBRARY",
            "QT_NO_CAST_FROM_ASCII",
            "QT_NO_CAST_TO_ASCII"
        ];
        if (project.linuxArchive)
            defs.push("TILED_LINUX_ARCHIVE");
        return defs;
    }

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.cxxFlags: ["-Wno-unknown-pragmas"]
    }

    bundle.isBundle: false
    cpp.sonamePrefix: qbs.targetOS.contains("darwin") ? "@rpath" : undefined

    files: [
        "compression.cpp",
        "compression.h",
        "gidmapper.cpp",
        "gidmapper.h",
        "hexagonalrenderer.cpp",
        "hexagonalrenderer.h",
        "imagelayer.cpp",
        "imagelayer.h",
        "imagereference.cpp",
        "imagereference.h",
        "isometricrenderer.cpp",
        "isometricrenderer.h",
        "layer.cpp",
        "layer.h",
        "logginginterface.h",
        "map.cpp",
        "map.h",
        "mapformat.h",
        "mapobject.cpp",
        "mapobject.h",
        "mapreader.cpp",
        "mapreader.h",
        "maprenderer.cpp",
        "maprenderer.h",
        "maptovariantconverter.cpp",
        "maptovariantconverter.h",
        "mapwriter.cpp",
        "mapwriter.h",
        "objectgroup.cpp",
        "objectgroup.h",
        "object.h",
        "orthogonalrenderer.cpp",
        "orthogonalrenderer.h",
        "plugin.cpp",
        "plugin.h",
        "pluginmanager.cpp",
        "pluginmanager.h",
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
        "tilesetformat.cpp",
        "tilesetformat.h",
        "varianttomapconverter.cpp",
        "varianttomapconverter.h",
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
            else if (qbs.targetOS.contains("darwin"))
                return "Tiled.app/Contents/Frameworks"
            else
                return "lib"
        }
        fileTagsFilter: "dynamiclibrary"
    }
}
