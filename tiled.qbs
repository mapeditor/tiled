import qbs 1.0

Project {
    name: "Tiled"

    qbsSearchPaths: "qbs"
    minimumQbsVersion: "1.4.2"

    property string version: qbs.getEnv("TILED_VERSION") || "0.15.0";

    references: [
        "dist/win",
        "src/automappingconverter",
        "src/libtiled",
        "src/plugins",
        "src/qtpropertybrowser",
        "src/terraingenerator",
        "src/tiled",
        "src/tmxrasterizer",
        "src/tmxviewer",
        "translations",
        "util/java/libtiled-java",
        "util/java/tmxviewer-java"
    ]
}
