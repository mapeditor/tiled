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

    Group {
        name: "Man page (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/man/man1"
        files: [ "../../man/tmxrasterizer.1" ]
    }
}
