import qbs 1.0

TiledPlugin {
    cpp.defines: ["JSON_LIBRARY"]

    files: [
        "json_global.h",
        "jsonplugin.cpp",
        "jsonplugin.h",
        "qjsonparser/json.cpp",
        "qjsonparser/json.h",
    ]
}
