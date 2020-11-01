import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["RPMAP_LIBRARY", "KARCHIVE_STATIC_DEFINE"])
    cpp.includePaths: ["../../KArchive/src"]

    Depends { name: "KArchive" }

    files: [
        "rpmap_global.h",
        "rpmapplugin.cpp",
        "rpmapplugin.h",
        "plugin.json",
    ]
}
