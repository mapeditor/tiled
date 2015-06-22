import qbs 1.0

QtGuiApplication {
    name: "tiledquick"
    targetName: name

    condition: Qt.core.versionMajor >= 5

    Depends { name: "Qt.core" }     // for the Qt version check
    Depends {
        condition: Qt.core.versionMajor >= 5
        name: "Qt"; submodules: ["quick", "widgets"]
    }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]

    files: [
        "main.cpp",
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

    Group {
        name: "QML files"
        qbs.install: true
        qbs.installDir: installBase + "qml/tiledquick"
        files: [
            "DragArea.qml",
            "main.qml",
        ]
    }
}
