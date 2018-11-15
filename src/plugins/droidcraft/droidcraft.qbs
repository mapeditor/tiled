import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["DROIDCRAFT_LIBRARY"])

    files: [
        "droidcraft_global.h",
        "droidcraftplugin.cpp",
        "droidcraftplugin.h",
        "droidcraft.qrc",
        "plugin.json",
    ]
}
