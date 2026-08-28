import qbs.FileInfo
import qbs.File
import qbs.TextFile
import qbs.Environment

WindowsInstallerPackage {
    builtByDefault: project.windowsInstaller
    condition: {
        if (project.windowsInstaller) {
            if (!(qbs.toolchain.contains("mingw") || qbs.toolchain.contains("msvc"))) {
                console.error("Unsupported configuration for Windows installer");
                return false;
            }
        }

        return project.windowsInstaller;
    }

    Depends { productTypes: ["application", "dynamiclibrary"] }

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }

    property string version: Environment.getEnv("TILED_MSI_VERSION") || project.version

    targetName: "Tiled-" + project.version + "_" + qbs.architecture

    wix.defines: {
        var defs = [
            "Version=" + version,
            "InstallRoot=" + qbs.installRoot,
            "QtDir=" + FileInfo.joinPaths(Qt.core.binPath, ".."),
            "QtVersionMajor=" + Qt.core.versionMajor,
            "QtVersionMinor=" + Qt.core.versionMinor,
            "RootDir=" + project.sourceDirectory
        ];

        if (qbs.toolchain.contains("mingw"))
            defs.push("MingwDir=" + FileInfo.joinPaths(cpp.toolchainInstallPath, ".."));
        else if (qbs.toolchain.contains("msvc")) {
            if (cpp.compilerVersionMajor >= 19) {
                defs.push("VcUniversalCRT=true");
                defs.push("VcInstallDir=" + cpp.toolchainInstallPath);
            } else {
                defs.push("VcInstallDir=" + FileInfo.joinPaths(cpp.toolchainInstallPath, "../.."));
            }
        }

        if (Qt.core.versionMinor >= 7)
            defs.push("WindowsStylePlugin=qmodernwindowsstyle.dll")
        else
            defs.push("WindowsStylePlugin=qwindowsvistastyle.dll")

        if (project.sentry)
            defs.push("Sentry");

        var pythonHome = Environment.getEnv("PYTHONHOME");
        if (pythonHome && File.exists(pythonHome))
            defs.push("Python");

        var rpMapEnabled = !qbs.toolchain.contains("msvc")
        if (rpMapEnabled)
            defs.push("RpMap");

        var imageFormatsPath = FileInfo.joinPaths(Qt.core.pluginPath, "imageformats")
        if (File.exists(FileInfo.joinPaths(imageFormatsPath, "qaseprite.dll")))
            defs.push("AsepriteImageFormatPlugin=qaseprite.dll");

        return defs;
    }

    wix.extensions: [
        "WixUIExtension"
    ]

    files: [
        "Custom_InstallDir.wxs",
        "Custom_InstallDirDlg.wxs",
        "installer.wxs"
    ]

    // This is a clever hack to make the rule that compiles the installer
    // depend on all installables, since that rule implicitly depends on
    // any "wxi" tagged products.
    Rule {
        multiplex: true
        inputsFromDependencies: ["installable"]

        Artifact {
            filePath: "dummy.wxi"
            fileTags: ["wxi"]
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.silent = true;
            cmd.sourceCode = function() {
                var tf;
                try {
                    tf = new TextFile(output.filePath, TextFile.WriteOnly);
                    tf.writeLine("<Include/>");
                } finally {
                    if (tf)
                        tf.close();
                }
            };
            return [cmd];
        }
    }
}
