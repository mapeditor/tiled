import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["JSON_LIBRARY"])

    files: [
        "json_global.h",
        "jsonplugin.cpp",
        "jsonplugin.h",
        "plugin.json",
        "qjsonparser/json.cpp",
        "qjsonparser/json.h",
    ]
}
