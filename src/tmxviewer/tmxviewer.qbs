import qbs 1.0

QtGuiApplication {
    name: "tmxviewer"
    destinationDirectory: "bin"

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["widgets"] }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]

    files: [
        "main.cpp",
        "tmxviewer.cpp",
        "tmxviewer.h",
    ]
}
