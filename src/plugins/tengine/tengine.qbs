import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["TENGINE_LIBRARY"])

    files: [
        "plugin.json",
        "tengine_global.h",
        "tengineplugin.cpp",
        "tengineplugin.h",
    ]
}
