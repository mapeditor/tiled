import qbs 1.0

QtGuiApplication {
    name: "automappingconverter"
    targetName: name

    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: ["widgets"] }

    cpp.includePaths: ["."]
    cpp.rpaths: qbs.targetOS.contains("darwin") ? ["@loader_path/../Frameworks"] : ["$ORIGIN/../lib"]
    cpp.cxxLanguageVersion: "c++11"

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
        condition: qbs.targetOS.contains("osx")
        targetName: "Automapping Converter"
    }

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return ""
            else if (qbs.targetOS.contains("darwin"))
                return "Tiled.app/Contents/MacOS"
            else
                return "bin"
        }
        fileTagsFilter: product.type
    }
}
