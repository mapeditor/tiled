import qbs 1.0

Project {
    qbsSearchPaths: "qbs"

    references: [
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
