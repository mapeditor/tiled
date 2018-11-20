import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["CSV_LIBRARY"])

    files: [
        "csv_global.h",
        "csvplugin.cpp",
        "csvplugin.h",
        "plugin.json",
    ]
}
