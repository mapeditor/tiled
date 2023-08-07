import qbs.Environment

Project {
    name: "Tiled"

    qbsSearchPaths: "qbs"
    minimumQbsVersion: "1.13"

    property string version: Environment.getEnv("TILED_VERSION") || "1.10.2";
    property bool snapshot: Environment.getEnv("TILED_SNAPSHOT") == "true"
    property bool release: Environment.getEnv("TILED_RELEASE") == "true"
    property string libDir: "lib"
    property bool installHeaders: false
    property bool useRPaths: true
    property bool windowsInstaller: false
    property bool windowsLayout: qbs.targetOS.contains("windows")
    property bool staticZstd: false
    property bool sentry: false
    property bool dbus: true
    property string openSslPath: Environment.getEnv("OPENSSL_PATH")

    references: [
        "dist/archive.qbs",
        "dist/distribute.qbs",
        "dist/win/installer.qbs",
        "docs",
        "src/karchive",
        "src/libtiled",
        "src/libtiledquick",
        "src/plugins",
        "src/qtpropertybrowser",
        "src/qtsingleapplication",
        "src/terraingenerator",
        "src/tiled/libtilededitor.qbs",
        "src/tiledapp",
        "src/tiledquick",
        "src/tiledquickplugin",
        "src/tmxrasterizer",
        "src/tmxviewer",
        "tests",
        "translations"
    ]

    AutotestRunner {
        name: "tests"
    }
}
