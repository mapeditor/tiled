import qbs 1.0

TiledPlugin {
    cpp.defines: ["TENGINE_LIBRARY"]

    files: [
        "tengine_global.h",
        "tengineplugin.cpp",
        "tengineplugin.h",
    ]
}
