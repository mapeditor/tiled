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
//        Environment.putEnv("QML2_IMPORT_PATH", "../../qml")
        // TODO: don't hard-code qml directory
        Environment.putEnv("QML2_IMPORT_PATH", "C:/dev/tiled-qt5_dev_debug-Debug/qtc_qt5_dev__07a3bce7-debug/install-root/qml")
        var path = Environment.getEnv("PATH");
        Environment.putEnv("PATH", path + ";C:/dev/tiled-qt5_dev_debug-Debug/qtc_qt5_dev__07a3bce7-debug/libtiled.qtc-qt5-dev--07a3bce7.dbe717e3");
    }

    files: [
        "tst_quicktiledplugin.cpp"
    ]
}
