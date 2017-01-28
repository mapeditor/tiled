import qbs 1.0

TiledPlugin {
    cpp.defines: ["ORX_LIBRARY"]

    files: [
        "orx_objects.h",
        "plugin.json",
        "orx_global.h",
        "orx_plugin.cpp",
        "orx_plugin.h",
    ]
}
