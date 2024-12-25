TiledQtGuiApplication {
    name: "terraingenerator"

    consoleApplication: true

    Depends { name: "libtiled" }

    cpp.includePaths: ["."]

    files: [
        "main.cpp",
    ]
}
