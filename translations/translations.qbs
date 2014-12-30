import qbs 1.0

Product {
    name: "translations"
    type: "qm"
    files: "*.ts"

    Depends { name: "Qt.core" }

    Group {
        fileTagsFilter: product.type
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return "translations"
            else
                return "share/tiled/translations"
        }
    }
}
