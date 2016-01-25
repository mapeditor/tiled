import qbs
import qbs.FileInfo
import qbs.File

WindowsInstallerPackage {
    builtByDefault: false
    condition: qbs.toolchain.contains("mingw") || qbs.toolchain.contains("msvc")

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

    targetName: "tiled-" + project.version + "-win" + bits

    wix.defines: {
        var defs = [
            "Version=" + project.version,
            "InstallRoot=" + qbs.installRoot,
            "QtDir=" + FileInfo.joinPaths(Qt.core.binPath, ".."),
            "RootDir=" + project.sourceDirectory
        ];

        if (qbs.toolchain.contains("mingw")) {
            defs.push("MingwDir=" + FileInfo.joinPaths(cpp.toolchainInstallPath, ".."));
        }

        // A bit of a hack to exclude the Python plugin when it isn't built
        if (File.exists("C:/Python27") &&
                qbs.toolchain.contains("mingw") &&
                !qbs.debugInformation) {
            defs.push("Python");
        }

        return defs;
    }

    wix.extensions: [
        "WixUIExtension"
    ]

    files: ["installer.wxs"]
}
