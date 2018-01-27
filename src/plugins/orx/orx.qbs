import qbs 1.0

TiledPlugin {
    cpp.defines: ["ORX_LIBRARY"]

    Depends { name: "Qt"; submodules: ["core", "widgets"]; versionAtLeast: "5.6" }

    files: [
        "name_generator.h",
        "optionsdialog.cpp",
        "optionsdialog.h",
        "optionsdialog.ui",
        "orx_cell_optimizer.cpp",
        "orx_cell_optimizer.h",
        "orx_exporter.cpp",
        "orx_exporter.h",
        "orx_object.cpp",
        "orx_object.h",
        "orx_objects.cpp",
        "orx_objects.h",
        "orx_utility.h",
        "plugin.json",
        "orx_global.h",
        "orx_plugin.cpp",
        "orx_plugin.h",
        "point_vector.h",
        "resources.qrc",
    ]
}
