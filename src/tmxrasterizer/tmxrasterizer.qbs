import qbs 1.0

TiledQtGuiApplication {
    name: "tmxrasterizer"

    consoleApplication: true

    Depends { name: "libtiled" }

    cpp.includePaths: ["."]

    files: [
        "main.cpp",
        "tmxrasterizer.cpp",
        "tmxrasterizer.h",
    ]
}
