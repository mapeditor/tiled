import qbs 1.0

TiledPlugin {
    cpp.defines: ["DROIDCRAFT_LIBRARY"]

    files: [
        "droidcraft_global.h",
        "droidcraftplugin.cpp",
        "droidcraftplugin.h",
        "droidcraft.qrc",
    ]
}
