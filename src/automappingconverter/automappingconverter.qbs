import qbs 1.0

TiledQtGuiApplication {
    name: "automappingconverter"
    targetName: name

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["widgets"] }

    cpp.includePaths: ["."]

    consoleApplication: false

    files: [
        "convertercontrol.cpp",
        "convertercontrol.h",
        "converterdatamodel.cpp",
        "converterdatamodel.h",
        "converterwindow.cpp",
        "converterwindow.h",
        "converterwindow.ui",
        "main.cpp",
    ]

    Properties {
        condition: qbs.targetOS.contains("macos")
        targetName: "Automapping Converter"
    }
}
