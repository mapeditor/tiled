Product {
    name: "translations"
    type: "qm"
    files: "*.ts"

    // Disabled languages because they're too incomplete
    excludeFiles: [
        "tiled_hi.ts",
        "tiled_lv.ts",
        "tiled_mr.ts",
    ]

    Depends { name: "Qt.core" }

    Group {
        fileTagsFilter: product.type
        qbs.install: true
        qbs.installDir: {
            if (project.windowsLayout)
                return "translations"
            else if (qbs.targetOS.contains("macos"))
                return "Tiled.app/Contents/Translations"
            else
                return "share/tiled/translations"
        }
    }
}
