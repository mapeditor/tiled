import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["TBIN_LIBRARY"])

    files: [
        "tbin_global.h",
        "tbinplugin.cpp",
        "tbinplugin.h",
        "plugin.json",
        "tbin/Layer.hpp",
        "tbin/Map.cpp",
        "tbin/Map.hpp",
        "tbin/PropertyValue.hpp",
        "tbin/Tile.hpp",
        "tbin/TileSheet.hpp",
        "tbin/FakeSfml.hpp",
    ]
}
