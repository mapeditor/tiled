import qbs 1.0

Project {
    qbsSearchPaths: "qbs"

    references: [
        "src/libtiled/libtiled.qbs",
        "src/plugins/plugins.qbs",
        "src/qtpropertybrowser/qtpropertybrowser.qbs",
        "src/tiled/tiled.qbs",
        "translations/translations.qbs",
    ]
}
