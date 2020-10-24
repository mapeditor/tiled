import qbs 1.0

TiledPlugin {
    cpp.defines: base.concat(["RPMAP_LIBRARY"])
    cpp.includePaths: ["/usr/include/KF5/KArchive"]
//    Depends { name: "KF5Archive" }
    cpp.dynamicLibraries: ["KF5Archive"]

    files: [
        "rpmap_global.h",
        "rpmapplugin.cpp",
        "rpmapplugin.h",
        "plugin.json",
    ]
}
