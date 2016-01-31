import qbs
import qbs.FileInfo
import qbs.File
import qbs.TextFile

WindowsInstallerPackage {
    builtByDefault: false
    condition: qbs.toolchain.contains("mingw") || qbs.toolchain.contains("msvc")

    Depends { productTypes: ["application", "dynamiclibrary"] }
    type: base.concat(["installable","appcast"])

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

        if (project.sparkleEnabled)
            defs.push("Sparkle");

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

    Group {
        name: "AppCastXml"
        files: [ "../appcast-win-snapshots.xml.in" ]
        fileTags: ["appCastXmlIn"]
    }

    Rule {
        inputs: ["appCastXmlIn"]
        Artifact {
            filePath: input.completeBaseName
            fileTags: "appcast"
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "prepare " + FileInfo.fileName(output.filePath);
            cmd.highlight = "codegen";

            cmd.sourceCode = function() {
                var i;
                var vars = {};
                var inf = new TextFile(input.filePath);
                var all = inf.readAll();

                vars['DATE'] = new Date().toISOString().slice(0, 10);
                vars['VERSION'] = project.version;
                vars['FILENAME'] = product.targetName + ".msi";

                for (i in vars) {
                    all = all.replace(new RegExp('@' + i + '@(?!\w)', 'g'), vars[i]);
                }

                var file = new TextFile(output.filePath, TextFile.WriteOnly);
                file.truncate();
                file.write(all);
                file.close();
            }

            return cmd;
        }
    }
}
