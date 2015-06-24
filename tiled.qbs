import qbs 1.0

Project {
    qbsSearchPaths: "qbs"

    property string version: qbs.getEnv("BUILD_INFO_VERSION")

    references: [
        "dist/win",
        "src/automappingconverter",
        "src/libtiled",
        "src/plugins",
        "src/qtpropertybrowser",
        "src/tiled",
        "src/tmxrasterizer",
        "src/tmxviewer",
        "translations",
    ]
}
