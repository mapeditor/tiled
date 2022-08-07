import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["EGM_LIBRARY"])

    files: [
        "egm_global.h",
        "egmplugin.cpp",
        "egmplugin.h",
        "plugin.json",
    ]

    qbsSearchPaths: "./"

    Depends { name: "libEGM" }
}
