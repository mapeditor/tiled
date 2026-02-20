TiledPlugin {
    cpp.defines: base.concat(["TBIN_LIBRARY"])

    files: [
        "tbin_global.h",
        "tbinplugin.cpp",
        "tbinplugin.h",
        "tbinmapformat.cpp",
        "tbinmapformat.h",
        "tidemapformat.cpp",
        "tidemapformat.h",
        "plugin.json",
        "tbin/Layer.hpp",
        "tbin/Map.cpp",
        "tbin/Map.hpp",
        "tbin/PropertyValue.hpp",
        "tbin/Tile.hpp",
        "tbin/TileSheet.hpp",
        "tbin/Vector2i.hpp",
    ]
}
