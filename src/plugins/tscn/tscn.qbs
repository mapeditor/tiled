TiledPlugin {
    cpp.defines: base.concat(["TSCN_LIBRARY"])

    files: [
        "tscn_global.h",
        "tscnplugin.cpp",
        "tscnplugin.h",
        "plugin.json",
    ]
}
