import qbs 1.0

Project {
    name: "Tiled"

    qbsSearchPaths: "qbs"
    minimumQbsVersion: "1.4.2"

    property string version: qbs.getEnv("TILED_VERSION") || "0.16.1";
    property bool sparkleEnabled: qbs.getEnv("TILED_SPARKLE")
    property bool snapshot: qbs.getEnv("TILED_SNAPSHOT")
    property bool release: qbs.getEnv("TILED_RELEASE")

    references: [
        "dist/win/archive.qbs",
        "dist/win/distribute.qbs",
        "dist/win/installer.qbs",
        "src/automappingconverter",
        "src/libtiled",
        "src/plugins",
        "src/qtpropertybrowser",
        "src/qtsingleapplication",
        "src/terraingenerator",
        "src/tiled",
        "src/tmxrasterizer",
        "src/tmxviewer",
        "translations",
        "util/java/libtiled-java",
        "util/java/tmxviewer-java"
    ]
}
