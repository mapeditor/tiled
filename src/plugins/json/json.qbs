import qbs 1.0

TiledPlugin {
    cpp.defines: ["JSON_LIBRARY"]

    files: [
        "json_global.h",
        "jsonplugin.cpp",
        "jsonplugin.h",
        "maptovariantconverter.cpp",
        "maptovariantconverter.h",
        "varianttomapconverter.cpp",
        "varianttomapconverter.h",
        "qjsonparser/json.cpp",
        "qjsonparser/json.h",
    ]
}
