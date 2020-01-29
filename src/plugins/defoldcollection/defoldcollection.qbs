import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["DEFOLDCOLLECTION_LIBRARY"])

    files: [
        "defoldcollectionplugin_global.h",
        "defoldcollectionplugin.cpp",
        "defoldcollectionplugin.h",
        "plugin.json",
    ]
}
