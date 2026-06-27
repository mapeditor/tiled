import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["ENIGMA_LIBRARY"])

    files: [
        "enigma_global.h",
        "enigmaplugin.cpp",
        "enigmaplugin.h",
        "plugin.json",
    ]

    qbsSearchPaths: "./"

    Depends { name: "libEnigma" }
}
