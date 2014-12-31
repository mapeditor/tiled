import qbs 1.0

TiledPlugin {
    cpp.defines: ["REPLICAISLAND_LIBRARY"]

    files: [
        "replicaisland_global.h",
        "replicaislandplugin.cpp",
        "replicaislandplugin.h",
        "replicaisland.qrc",
    ]
}
