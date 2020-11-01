import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["RPMAP_LIBRARY", "KARCHIVE_STATIC_DEFINE"])
    cpp.includePaths: ["../../KArchive/src"]

    Properties {
        condition: !qbs.toolchain.contains("msvc")
        cpp.dynamicLibraries: base.concat(["z"])
    }

    Properties {
        condition: qbs.targetOS.contains("darwin")
        bundle.isBundle: false
        cpp.sonamePrefix: "@rpath"
    }

    Depends { name: "KArchive" }

    files: [
        "rpmap_global.h",
        "rpmapplugin.cpp",
        "rpmapplugin.h",
        "plugin.json",
    ]
}
