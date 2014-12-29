import qbs 1.0

Project {
    qbsSearchPaths: "qbs"

    references: [
        "src/automappingconverter/automappingconverter.qbs",
        "src/libtiled/libtiled.qbs",
        "src/plugins/plugins.qbs",
        "src/qtpropertybrowser/qtpropertybrowser.qbs",
        "src/tiled/tiled.qbs",
        "src/tmxrasterizer/tmxrasterizer.qbs",
        "src/tmxviewer/tmxviewer.qbs",
        "translations/translations.qbs",
    ]
}
