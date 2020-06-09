import qbs 1.0

Product {
    name: "translations"
    type: "qm"
    files: "*.ts"

    // Disabled languages because they're too incomplete
    excludeFiles: [
        "tiled_hi.ts",
        "tiled_lv.ts",
        "tiled_mr.ts",
        "tiled_th.ts",
    ]

    Depends { name: "Qt.core" }

    Group {
        fileTagsFilter: product.type
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows") || project.linuxArchive)
                return "translations"
            else if (qbs.targetOS.contains("macos"))
                return "Tiled.app/Contents/Translations"
            else
                return "share/tiled/translations"
        }
    }
}
