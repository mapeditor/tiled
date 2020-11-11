import qbs 1.0

TiledPlugin {
    condition: Qt.core.versionMajor > 5 || Qt.core.versionMinor >= 12

    Depends { name: "Qt.core" }
    Depends { name: "karchive" }

    cpp.defines: base.concat(["RPMAP_LIBRARY"])

    Properties {
        condition: !qbs.toolchain.contains("msvc")
        cpp.dynamicLibraries: base.concat(["z"])
    }

    files: [
        "rpmap_global.h",
        "rpmapplugin.cpp",
        "rpmapplugin.h",
        "plugin.json",
    ]
}
