import qbs 1.0

QtGuiApplication {
    name: "tmxrasterizer"
    destinationDirectory: "bin"

    Depends { name: "libtiled" }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]

    files: [
        "main.cpp",
        "tmxrasterizer.cpp",
        "tmxrasterizer.h",
    ]
}
