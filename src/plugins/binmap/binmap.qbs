TiledPlugin {
    cpp.defines: base.concat(["BINMAP_LIBRARY"])

    files: [
        "binmap_global.h",
        "binmapplugin.cpp",
        "binmapplugin.h",
        "plugin.json",
    ]
}
