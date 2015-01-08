import qbs 1.0

QtGuiApplication {
    name: "tmxrasterizer"

    consoleApplication: true

    Depends { name: "libtiled" }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]

    files: [
        "main.cpp",
        "tmxrasterizer.cpp",
        "tmxrasterizer.h",
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
