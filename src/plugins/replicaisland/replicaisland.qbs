import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["REPLICAISLAND_LIBRARY"])

    files: [
        "plugin.json",
        "replicaisland_global.h",
        "replicaislandplugin.cpp",
        "replicaislandplugin.h",
        "replicaisland.qrc",
    ]
}
