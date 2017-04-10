import qbs 1.0

QtGuiApplication {
    name: "tiledquick"
    targetName: name

    Depends {
        name: "Qt"
        submodules: ["core", "quick", "widgets"]
        versionAtLeast: "5.4"
    }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]
    cpp.cxxLanguageVersion: "c++11"

    files: [
        "main.cpp",
        "qml.qrc",
    ]

    Properties {
        condition: qbs.targetOS.contains("osx")
        targetName: "Tiled Quick"
    }

    property string installBase: qbs.targetOS.contains("osx") ? "Tiled Quick.app/Contents/" : ""

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows") || qbs.targetOS.contains("osx"))
                return ""
            else
                return "bin"
        }
        fileTagsFilter: product.type
    }
}
