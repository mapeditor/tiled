import qbs 1.0

Product {
    name: "translations"
    type: "qm"
    files: "*.ts"

    Depends { name: "Qt.core" }

    Group {
        fileTagsFilter: product.type
        qbs.install: true
        qbs.installDir: "share/tiled/translations"
    }
}
