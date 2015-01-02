import qbs 1.0

QtGuiApplication {
    name: "tmxviewer"

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["widgets"] }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]

    consoleApplication: false

    files: [
        "main.cpp",
        "tmxviewer.cpp",
        "tmxviewer.h",
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
