import qbs 1.0

QtGuiApplication {
    name: "tiledquick"

    Depends { name: "Qt"; submodules: ["qml", "quick", "widgets"] }

    cpp.includePaths: ["."]
    cpp.rpaths: ["$ORIGIN/../lib"]

    files: [
        "main.cpp",
        "qml.qrc",
    ]

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return ""
            else
                return "bin"
        }
        fileTagsFilter: "application"
    }
}
