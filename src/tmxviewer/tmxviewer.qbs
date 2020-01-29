import qbs 1.0

TiledQtGuiApplication {
    name: "tmxviewer"

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["widgets"]; versionAtLeast: "5.6" }

    cpp.includePaths: ["."]

    consoleApplication: false

    files: [
        "main.cpp",
        "tmxviewer.cpp",
        "tmxviewer.h",
    ]

    Group {
        name: "Man page (Linux)"
        condition: qbs.targetOS.contains("linux")
        qbs.install: true
        qbs.installDir: "share/man/man1"
        files: [ "../../man/tmxviewer.1" ]
    }
}
