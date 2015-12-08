import qbs 1.0

TiledPlugin {
    cpp.defines: ["TMW_LIBRARY"]

    files: [
        "plugin.json",
        "tmw_global.h",
        "tmwplugin.cpp",
        "tmwplugin.h",
    ]
}
