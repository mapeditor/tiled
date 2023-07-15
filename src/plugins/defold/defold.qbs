TiledPlugin {
    cpp.defines: base.concat(["DEFOLD_LIBRARY"])

    files: [
        "defoldplugin_global.h",
        "defoldplugin.cpp",
        "defoldplugin.h",
        "plugin.json",
    ]
}
