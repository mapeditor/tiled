import qbs 1.0
import qbs.Environment

Project {
    name: "Tiled"

    qbsSearchPaths: "qbs"
    minimumQbsVersion: "1.8"

    property string version: Environment.getEnv("TILED_VERSION") || "1.5.0";
    property bool snapshot: Environment.getEnv("TILED_SNAPSHOT") == "true"
    property bool release: Environment.getEnv("TILED_RELEASE") == "true"
    property bool installHeaders: false
    property bool useRPaths: true
    property bool windowsInstaller: false
    property bool enableZstd: false
    property string openSslPath: Environment.getEnv("OPENSSL_PATH")

    references: [
        "dist/archive.qbs",
        "dist/distribute.qbs",
        "dist/win/installer.qbs",
        "docs",
        "src/libtiled",
        "src/libtiledquick",
        "src/karchive",
        "src/plugins",
        "src/qtpropertybrowser",
        "src/qtsingleapplication",
        "src/terraingenerator",
        "src/tiled",
        "src/tiledquick",
        "src/tiledquickplugin",
        "src/tmxrasterizer",
        "src/tmxviewer",
        "tests",
        "translations"
    ]
}
