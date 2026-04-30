import qbs.Utilities

DynamicLibrary {
    targetName: "tiledquick"
    builtByDefault: false
    condition: Qt.core && Utilities.versionCompare(Qt.core.version, "6.5") >= 0

    Depends { name: "libtiled" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["quick","shadertools"]; versionAtLeast: "6.5" }

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
    cpp.defines: [
        "TILED_QUICK_LIBRARY",
        "QT_NO_CAST_FROM_ASCII",
        "QT_NO_CAST_TO_ASCII",
        "QT_NO_URL_CAST_FROM_STRING",
        "QT_DISABLE_DEPRECATED_BEFORE=0x060500",
        "QT_NO_DEPRECATED_WARNINGS",
        "QT_NO_FOREACH"
    ]

    Properties {
        condition: qbs.targetOS.contains("darwin")
        bundle.isBundle: false
        cpp.sonamePrefix: "@rpath"
    }

    files: [
        "mapborderitem.cpp",
        "mapborderitem.h",
        "mapgriditem.cpp",
        "mapgriditem.h",
        "mapgridmaterial.cpp",
        "mapgridmaterial.h",
        "mapitem.cpp",
        "mapitem.h",
        "maploader.cpp",
        "maploader.h",
        "mapref.h",
        "tiledquick_global.h",
        "tilelayeritem.cpp",
        "tilelayeritem.h",
        "tilesnode.cpp",
        "tilesnode.h",
    ]

    Group {
        condition: project.installHeaders
        qbs.install: true
        qbs.installDir: "include/tiledquick"
        fileTagsFilter: "hpp"
    }

    Group {
        name: "Shaders"
        files: [
            "grid.vert",
            "grid.frag",
        ]
    }

    Export {
        Depends { name: "cpp" }
        Depends {
            name: "Qt"
            submodules: ["quick"]
        }

        cpp.includePaths: exportingProduct.sourceDirectory
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
