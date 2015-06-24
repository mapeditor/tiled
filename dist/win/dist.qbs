import qbs
import qbs.FileInfo

NSISSetup {
    builtByDefault: false
    condition: qbs.toolchain.contains("mingw")

    Depends { productTypes: ["application", "dynamiclibrary"] }
    type: base.concat(["installable"])

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }

    property int bits: {
        if (qbs.architecture === "x86_64")
            return 64;
        if (qbs.architecture === "x86")
            return 32;
    }

    targetName: "tiled-" + project.version + "-win" + bits + "-setup"

    nsis.defines: [
        "QT_DIR=" + FileInfo.joinPaths(Qt.core.binPath, ".."),
        "MINGW_DIR=" + FileInfo.joinPaths(cpp.toolchainInstallPath, ".."),
        "V=" + project.version,
        "ARCH=" + qbs.architecture,
        "ROOT_DIR=" + project.sourceDirectory,
        "BUILD_DIR=" + qbs.installRoot
    ]

    files: [
        "*"
    ]
}
