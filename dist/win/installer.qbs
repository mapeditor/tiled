import qbs
import qbs.FileInfo
import qbs.File
import qbs.TextFile
import qbs.Environment

WindowsInstallerPackage {
    builtByDefault: false
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

    property int bits: {
        if (qbs.architecture === "x86_64")
            return 64;
        if (qbs.architecture === "x86")
            return 32;
    }

    targetName: "Tiled-" + project.version + "-win" + bits

    wix.defines: {
        var defs = [
            "Version=" + project.version,
            "InstallRoot=" + qbs.installRoot,
            "QtDir=" + FileInfo.joinPaths(Qt.core.binPath, ".."),
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

        if (Qt.core.versionMinor >= 10)
            defs.push("WindowsVistaStyle")

        if (File.exists(Environment.getEnv("PYTHONHOME")))
            defs.push("Python");

        var openSslDir = "C:\\OpenSSL-Win" + bits;
        if (File.exists(openSslDir))
            defs.push("OpenSslDir=" + openSslDir);

        return defs;
    }

    wix.extensions: [
        "WixUIExtension"
    ]

    files: ["installer.wxs"]

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
