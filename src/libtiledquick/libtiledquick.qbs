DynamicLibrary {
    targetName: "tiledquick"
    builtByDefault: false

    Depends { name: "libtiled" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["quick"]; versionAtLeast: "5.12" }

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
        "QT_DISABLE_DEPRECATED_BEFORE=QT_VERSION_CHECK(5,15,0)",
        "QT_NO_DEPRECATED_WARNINGS",
        "QT_NO_FOREACH"
    ]

    Properties {
        condition: qbs.targetOS.contains("darwin")
        bundle.isBundle: false
        cpp.sonamePrefix: "@rpath"
    }

    files: [
        "mapitem.h",
        "mapitem.cpp",
        "maploader.h",
        "maploader.cpp",
        "mapref.h",
        "tilelayeritem.h",
        "tilelayeritem.cpp",
        "tiledquick_global.h",
        "tilesnode.h",
        "tilesnode.cpp"
    ]

    Group {
        condition: project.installHeaders
        qbs.install: true
        qbs.installDir: "include/tiledquick"
        fileTagsFilter: "hpp"
    }

    Export {
        Depends { name: "cpp" }
        Depends {
            name: "Qt"
            submodules: ["quick"]
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
