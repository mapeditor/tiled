import qbs 1.0

TiledPlugin {
    cpp.defines: ["ORX_LIBRARY"]

    files: [
        "orx_exporter.cpp",
        "orx_exporter.h",
        "orx_objects.cpp",
        "orx_objects.h",
        "plugin.json",
        "orx_global.h",
        "orx_plugin.cpp",
        "orx_plugin.h",
    ]
}
