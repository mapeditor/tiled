import qbs.Environment

Project {
    name: "Tiled"

    qbsSearchPaths: "qbs"
    // Qbs 1.23 adds the required /permissive- flag for MSVC automatically.
    minimumQbsVersion: qbs.toolchain.contains("msvc") ? "1.23" : "1.18"

    property string version: Environment.getEnv("TILED_VERSION") || "1.12.2";
    property bool snapshot: Environment.getEnv("TILED_SNAPSHOT") == "true"
    property bool release: Environment.getEnv("TILED_RELEASE") == "true"
    property string libDir: "lib"
    property bool installHeaders: false
    property bool useRPaths: true
    property bool windowsInstaller: false
    property bool windowsLayout: qbs.targetOS.contains("windows")
    property bool staticZstd: false
    property bool sentry: false
    property bool dbus: true
    property string openSslPath: Environment.getEnv("OPENSSL_PATH")
    property string pythonPkgConfigName: "python3-embed"

    // The QML modules shipped with Tiled for use by QML extensions, along
    // with the Qt libraries they need at runtime. Used by the "distribute"
    // and "installer" products (on Linux and macOS, the deployment tools
    // derive this from dist/qml/imports.qml instead). Only the files directly
    // in each listed directory are shipped.
    property stringList qmlImportDirs: [
        "QML",
        "QtQml",
        "QtQml/Models",
        "QtQml/WorkerScript",
        "QtQuick",
        "QtQuick/Controls",
        "QtQuick/Controls/Basic",
        "QtQuick/Controls/Basic/impl",
        "QtQuick/Controls/Fusion",
        "QtQuick/Controls/Fusion/impl",
        "QtQuick/Controls/impl",
        "QtQuick/Layouts",
        "QtQuick/Templates",
        "QtQuick/Window",
    ]
    property stringList qtQuickLibraries: [
        "QmlMeta",
        "QmlModels",
        "QmlWorkerScript",
        "Quick",
        "QuickControls2",
        "QuickControls2Basic",
        "QuickControls2BasicStyleImpl",
        "QuickControls2Fusion",
        "QuickControls2FusionStyleImpl",
        "QuickControls2Impl",
        "QuickLayouts",
        "QuickTemplates2",
        "QuickWidgets",
    ]

    references: [
        "dist/archive.qbs",
        "dist/distribute.qbs",
        "dist/win/installer.qbs",
        "docs",
        "src/karchive",
        "src/libtiled",
        "src/libtiledquick",
        "src/plugins",
        "src/qtsingleapplication",
        "src/terraingenerator",
        "src/tiled/libtilededitor.qbs",
        "src/tiledapp",
        "src/tiledquick",
        "src/tiledquickplugin",
        "src/tmxrasterizer",
        "src/tmxviewer",
        "tests",
        "translations"
    ]

    AutotestRunner {
        name: "tests"
    }
}
