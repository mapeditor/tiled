import qbs
import qbs.Environment

QtGuiApplication {
    name: "tst_quicktiledplugin"
    type: ["application", "autotest"]
    targetName: name

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["gui", "testlib", "quick"] }

    cpp.dynamicLibraries: {
        var suffix = qbs.targetOS.contains("windows") && qbs.buildVariant === "debug" ? "d" : "";
        return [ "Qt5QuickTest" + suffix ];
    }

    qbs.setupRunEnvironment: {
        // TODO: don't hard-code qml directory
        Environment.putEnv("QML2_IMPORT_PATH", "C:/dev/tiled-qt5_dev_debug-Debug/qtc_qt5_dev__07a3bce7-debug/install-root/qml")

        if (targetOS.contains('windows')) {
            var PATH = Environment.getEnv("PATH");
//            Environment.putEnv("PATH", PATH + ";" + buildDir + "/install-root");
            Environment.putEnv("PATH", PATH + ";" + "C:/dev/tiled-qt5_dev_debug-Debug/qtc_qt5_dev__07a3bce7-debug/install-root");
            console.info(Environment.getEnv("PATH"))
        }
    }

    files: [
        "tst_quicktiledplugin.cpp"
    ]
}
