TiledPlugin {
    cpp.defines: ["RPD_LIBRARY"]

    files: [
        "rpd_global.h",
        "rpdplugin.cpp",
        "rpdplugin.h",
        "plugin.json",
        "qjsonparser/json.cpp",
        "qjsonparser/json.h",
    ]
}
