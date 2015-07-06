import qbs 1.0

Project {
    qbsSearchPaths: "qbs"

    property string version: qbs.getEnv("VERSION")

    references: [
        "dist/win",
        "src/automappingconverter",
        "src/libtiled",
        "src/plugins",
        "src/qtpropertybrowser",
        "src/tiled",
        "src/tiledquick",
        "src/tiledquickplugin",
        "src/tmxrasterizer",
        "src/tmxviewer",
        "translations",
    ]
}
