import qbs 1.0

TiledQtGuiApplication {
    name: "terraingenerator"

    consoleApplication: true

    Depends { name: "libtiled" }

    cpp.includePaths: ["."]
    cpp.defines: ["QT_NO_FOREACH"]

    files: [
        "main.cpp",
    ]
}
