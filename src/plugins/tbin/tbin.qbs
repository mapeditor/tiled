TiledPlugin {
    cpp.defines: base.concat([
        "TBIN_LIBRARY",
        "QT_NO_CAST_FROM_ASCII",
        "QT_NO_CAST_TO_ASCII",
    ])

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
