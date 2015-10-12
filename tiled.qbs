import qbs 1.0

Project {
    qbsSearchPaths: "qbs"
    minimumQbsVersion: "1.4.2"

    property string version: qbs.getEnv("VERSION")

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
