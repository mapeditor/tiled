import qbs 1.0

TiledPlugin {
    cpp.defines: ["LUA_LIBRARY"]

    files: [
        "lua_global.h",
        "luaplugin.cpp",
        "luaplugin.h",
        "luatablewriter.cpp",
        "luatablewriter.h",
    ]
}
