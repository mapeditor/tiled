import qbs 1.0

TiledPlugin {
    cpp.defines: ["ORX_LIBRARY"]

    Depends { name: "Qt"; submodules: ["core", "widgets"]; versionAtLeast: "5.6" }

    files: [
        "optionsdialog.cpp",
        "optionsdialog.h",
        "optionsdialog.ui",
        "orx_exporter.cpp",
        "orx_exporter.h",
        "orx_object.cpp",
        "orx_object.h",
        "orx_objects.cpp",
        "orx_objects.h",
        "plugin.json",
        "orx_global.h",
        "orx_plugin.cpp",
        "orx_plugin.h",
        "point_vector.h",
        "resources.qrc",
    ]
}
