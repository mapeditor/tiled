import qbs 1.0

QtGuiApplication {
    name: "automappingconverter"
    destinationDirectory: "bin"

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["widgets"] }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]

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

    Group {
        qbs.install: true
        qbs.installDir: "bin"
        fileTagsFilter: "application"
    }
}
