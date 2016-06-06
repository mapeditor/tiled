import qbs 1.0
import qbs.Environment

Project {
    name: "Tiled"

    qbsSearchPaths: "qbs"
    minimumQbsVersion: "1.5.0"

    property string version: Environment.getEnv("TILED_VERSION") || "0.16.1";
    property bool sparkleEnabled: Environment.getEnv("TILED_SPARKLE")
    property bool snapshot: Environment.getEnv("TILED_SNAPSHOT")
    property bool release: Environment.getEnv("TILED_RELEASE")

    references: [
        "dist/win/dist.qbs",
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
