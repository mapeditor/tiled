TiledPlugin {
    condition: !qbs.toolchain.contains("msvc")

    Depends { name: "Qt.core" }
    Depends { name: "karchive" }

    cpp.defines: base.concat(["RPMAP_LIBRARY"])
    cpp.dynamicLibraries: base.concat(["z"])

    files: [
        "rpmap_global.h",
        "rpmapplugin.cpp",
        "rpmapplugin.h",
        "plugin.json",
    ]
}
