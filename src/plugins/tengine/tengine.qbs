TiledPlugin {
    cpp.defines: base.concat(["TENGINE_LIBRARY"])

    files: [
        "plugin.json",
        "tengine_global.h",
        "tengineplugin.cpp",
        "tengineplugin.h",
    ]
}
