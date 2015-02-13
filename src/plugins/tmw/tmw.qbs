import qbs 1.0

TiledPlugin {
    cpp.defines: ["TMW_LIBRARY"]

    files: [
        "tmw_global.h",
        "tmwplugin.cpp",
        "tmwplugin.h",
    ]
}
