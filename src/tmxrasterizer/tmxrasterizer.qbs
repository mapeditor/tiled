import qbs 1.0

QtGuiApplication {
    name: "tmxrasterizer"

    consoleApplication: true

    Depends { name: "libtiled" }

    cpp.includePaths: ["."]
    cpp.rpaths: qbs.targetOS.contains("darwin") ? ["@loader_path/../Frameworks"] : ["$ORIGIN/../lib"]
    cpp.cxxLanguageVersion: "c++11"

    files: [
        "main.cpp",
        "tmxrasterizer.cpp",
        "tmxrasterizer.h",
    ]

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return ""
            else if (qbs.targetOS.contains("darwin"))
                return "Tiled.app/Contents/MacOS"
            else
                return "bin"
        }
        fileTagsFilter: product.type
    }
}
