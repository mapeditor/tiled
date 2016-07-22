import qbs 1.0

TiledPlugin {
    cpp.defines: ["DEFOLD_LIBRARY"]

    files: [
        "defoldplugin_global.h",
        "defoldplugin.cpp",
        "defoldplugin.h",
        "plugin.json",
        "tokendefines.h",
        "luaplugin.cpp",
        "luaplugin.h",
        "luatablewriter.cpp",
        "luatablewriter.h",
    ]
}
