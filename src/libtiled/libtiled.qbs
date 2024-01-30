import qbs.Probes as Probes

DynamicLibrary {
    targetName: "tiled"
    cpp.dynamicLibraryPrefix: "lib"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui"; versionAtLeast: "5.12" }

    Probes.PkgConfigProbe {
        id: pkgConfigZstd
        name: "libzstd"
        forStaticBuild: project.staticZstd
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
    cpp.visibility: "minimal"
    cpp.defines: {
        var defs = [
            "TILED_LIBRARY",
            "TILED_LIB_DIR=\"" + project.libDir + "\"",
            "QT_NO_CAST_FROM_ASCII",
            "QT_NO_CAST_TO_ASCII",
            "QT_NO_URL_CAST_FROM_STRING",
            "QT_DISABLE_DEPRECATED_BEFORE=QT_VERSION_CHECK(5,15,0)",
            "QT_NO_DEPRECATED_WARNINGS",
            "_USE_MATH_DEFINES",
        ]

        if (project.staticZstd || pkgConfigZstd.found)
            defs.push("TILED_ZSTD_SUPPORT");

        if (project.windowsLayout)
            defs.push("TILED_WINDOWS_LAYOUT");

        return defs;
    }
    cpp.dynamicLibraries: {
        var libs = base;

        if (!qbs.toolchain.contains("msvc"))
            libs.push("z");

        if (pkgConfigZstd.found && !project.staticZstd)
            libs = libs.concat(pkgConfigZstd.libraries);

        return libs;
    }
    cpp.staticLibraries: {
        var libs = base;

        if (project.staticZstd) {
            if (pkgConfigZstd.found)
                libs = libs.concat(pkgConfigZstd.libraries);
            else
                libs.push("zstd");
        }

        return libs;
    }

    Properties {
        condition: pkgConfigZstd.found
        cpp.cxxFlags: outer.concat(pkgConfigZstd.cflags)
        cpp.libraryPaths: outer.concat(pkgConfigZstd.libraryPaths)
        cpp.linkerFlags: outer.concat(pkgConfigZstd.linkerFlags)
    }

    // When libzstd was not found but staticZstd is enabled, assume that zstd
    // has been compiled in a "zstd" directory at the repository root (this is
    // done by the autobuilds for Windows and macOS).
    Properties {
        condition: !pkgConfigZstd.found && project.staticZstd
        cpp.libraryPaths: outer.concat(["../../zstd/lib"])
        cpp.includePaths: outer.concat(["../../zstd/lib"])
    }

    Properties {
        condition: qbs.targetOS.contains("darwin")
        bundle.isBundle: false
        cpp.sonamePrefix: "@rpath"
    }

    files: [
        "compression.cpp",
        "compression.h",
        "containerhelpers.h",
        "fileformat.cpp",
        "fileformat.h",
        "filesystemwatcher.cpp",
        "filesystemwatcher.h",
        "gidmapper.cpp",
        "gidmapper.h",
        "grid.h",
        "grouplayer.cpp",
        "grouplayer.h",
        "hex.cpp",
        "hex.h",
        "hexagonalrenderer.cpp",
        "hexagonalrenderer.h",
        "imagecache.cpp",
        "imagecache.h",
        "imagelayer.cpp",
        "imagelayer.h",
        "imagereference.cpp",
        "imagereference.h",
        "isometricrenderer.cpp",
        "isometricrenderer.h",
        "layer.cpp",
        "layer.h",
        "logginginterface.cpp",
        "logginginterface.h",
        "map.cpp",
        "map.h",
        "mapformat.cpp",
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
        "minimaprenderer.cpp",
        "minimaprenderer.h",
        "object.cpp",
        "object.h",
        "objectgroup.cpp",
        "objectgroup.h",
        "objecttemplate.cpp",
        "objecttemplate.h",
        "objecttemplateformat.cpp",
        "objecttemplateformat.h",
        "objecttypes.cpp",
        "objecttypes.h",
        "orthogonalrenderer.cpp",
        "orthogonalrenderer.h",
        "plugin.cpp",
        "plugin.h",
        "pluginmanager.cpp",
        "pluginmanager.h",
        "properties.cpp",
        "properties.h",
        "propertytype.cpp",
        "propertytype.h",
        "savefile.cpp",
        "savefile.h",
        "staggeredrenderer.cpp",
        "staggeredrenderer.h",
        "templatemanager.cpp",
        "templatemanager.h",
        "tile.cpp",
        "tileanimationdriver.cpp",
        "tileanimationdriver.h",
        "tiled.cpp",
        "tiled_global.h",
        "tiled.h",
        "tile.h",
        "tilelayer.cpp",
        "tilelayer.h",
        "tileset.cpp",
        "tileset.h",
        "tilesetformat.cpp",
        "tilesetformat.h",
        "tilesetmanager.cpp",
        "tilesetmanager.h",
        "varianttomapconverter.cpp",
        "varianttomapconverter.h",
        "wangset.cpp",
        "wangset.h",
        "world.cpp",
        "world.h",
    ]

    Group {
        condition: project.installHeaders
        qbs.install: true
        qbs.installDir: "include/tiled"
        fileTagsFilter: "hpp"
    }

    Export {
        Depends { name: "cpp" }
        Depends {
            name: "Qt"
            submodules: ["gui"]
        }

        cpp.includePaths: "."
    }

    install: !qbs.targetOS.contains("darwin")
    installDir: {
        if (qbs.targetOS.contains("windows"))
            if (project.windowsLayout)
                return ""
            else
                return "bin"
        else
            return project.libDir
    }
}
