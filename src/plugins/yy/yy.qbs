import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["YY_LIBRARY"])

    files: [
        "jsonwriter.cpp",
        "jsonwriter.h",
        "plugin.json",
        "yy_global.h",
        "yyplugin.cpp",
        "yyplugin.h",
    ]
}
