import qbs 1.0

TiledQtGuiApplication {
    name: "tmxtojson"

    consoleApplication: true

    Depends { name: "libtiled" }

    cpp.includePaths: ["."]

    files: [
        "main.cpp"
    ]
}
