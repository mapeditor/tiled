import qbs 1.0

QtGuiApplication {
    name: "terraingenerator"

    consoleApplication: true

    Depends { name: "libtiled" }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]

    files: [
        "main.cpp",
    ]

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows") || qbs.targetOS.contains("osx"))
                return ""
            else
                return "bin"
        }
        fileTagsFilter: product.type
    }
}
