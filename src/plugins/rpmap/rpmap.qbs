import qbs.Utilities

TiledPlugin {
    condition: {
        if (qbs.toolchain.contains("msvc"))
            return false;

        if (Utilities.versionCompare(Qt.core.version, "6.8") < 0)
            return false;

        return true;
    }

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
