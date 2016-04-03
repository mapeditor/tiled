import qbs 1.0

TiledQtGuiApplication {
    name: "tmxviewer"

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["widgets"] }

    cpp.includePaths: ["."]

    consoleApplication: false

    files: [
        "main.cpp",
        "tmxviewer.cpp",
        "tmxviewer.h",
    ]
}
