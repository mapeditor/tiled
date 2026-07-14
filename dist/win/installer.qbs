import qbs.FileInfo
import qbs.File
import qbs.TextFile
import qbs.Environment
import qbs.Utilities

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

    // Generates a WiX fragment that ships the QML modules usable by QML
    // extensions, along with the Qt libraries they need. The fragment is
    // generated based on the actual contents of the Qt installation, since
    // the set of files differs between Qt versions.
    Rule {
        multiplex: true
        inputsFromDependencies: ["installable"]

        Artifact {
            filePath: "qml-modules.wxs"
            fileTags: ["wxs"]
        }

        prepare: {
            var binPath = product.moduleProperty("Qt.core", "binPath");

            var cmd = new JavaScriptCommand();
            cmd.description = "generating qml-modules.wxs";
            cmd.qmlDir = FileInfo.joinPaths(binPath, "../qml");
            cmd.binDir = binPath;
            cmd.qmlImportDirs = project.qmlImportDirs;
            cmd.qtQuickLibraries = project.qtQuickLibraries;
            cmd.versionMajor = product.moduleProperty("Qt.core", "versionMajor");
            cmd.sourceCode = function() {
                // WiX identifiers are limited to 72 characters
                function idFor(prefix, path) {
                    var id = prefix + path.replace(/[^A-Za-z0-9_.]/g, "_");
                    if (id.length > 72)
                        id = id.slice(0, 30) + "_" + Utilities.getHash(path);
                    return id;
                }

                var tf;
                try {
                    tf = new TextFile(output.filePath, TextFile.WriteOnly);
                    tf.writeLine("<?xml version='1.0' encoding='windows-1252'?>");
                    tf.writeLine("<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>");
                    tf.writeLine("  <Fragment>");

                    // Directory tree below INSTALLDIR/qml
                    var tree = {};
                    var i, j;
                    for (i = 0; i < qmlImportDirs.length; ++i) {
                        var parts = qmlImportDirs[i].split("/");
                        var node = tree;
                        for (j = 0; j < parts.length; ++j)
                            node = node[parts[j]] = node[parts[j]] || {};
                    }

                    function writeDirs(node, relPath, indent) {
                        for (var name in node) {
                            var rel = relPath ? relPath + "/" + name : name;
                            tf.writeLine(indent + "<Directory Id='" + idFor("qmldir_", rel) + "' Name='" + name + "'>");
                            writeDirs(node[name], rel, indent + "  ");
                            tf.writeLine(indent + "</Directory>");
                        }
                    }

                    tf.writeLine("    <DirectoryRef Id='INSTALLDIR'>");
                    tf.writeLine("      <Directory Id='qmldir' Name='qml'>");
                    writeDirs(tree, "", "        ");
                    tf.writeLine("      </Directory>");
                    tf.writeLine("    </DirectoryRef>");

                    tf.writeLine("    <ComponentGroup Id='QmlModules'>");

                    for (i = 0; i < qmlImportDirs.length; ++i) {
                        var dir = qmlImportDirs[i];
                        var absDir = FileInfo.joinPaths(qmlDir, dir);
                        if (!File.exists(absDir))
                            continue;

                        var entries = File.directoryEntries(absDir, File.Files);
                        for (j = 0; j < entries.length; ++j) {
                            var entry = entries[j];
                            if (entry.endsWith(".qmltypes") || entry.endsWith(".pdb"))
                                continue;

                            // Skip debug variants of the QML plugins
                            if (entry.endsWith("d.dll") && File.exists(FileInfo.joinPaths(absDir, entry.slice(0, -5) + ".dll")))
                                continue;

                            var rel = dir + "/" + entry;
                            tf.writeLine("      <Component Id='" + idFor("cmp_", rel) + "' Directory='" + idFor("qmldir_", dir) + "' Guid='*'>");
                            tf.writeLine("        <File Id='" + idFor("qml_", rel) + "' Source='" + FileInfo.toWindowsSeparators(FileInfo.joinPaths(absDir, entry)) + "' KeyPath='yes' />");
                            tf.writeLine("      </Component>");
                        }
                    }

                    // Qt libraries needed by the QML modules. Their existence
                    // depends on the Qt version.
                    for (i = 0; i < qtQuickLibraries.length; ++i) {
                        var dll = "Qt" + versionMajor + qtQuickLibraries[i] + ".dll";
                        var absFile = FileInfo.joinPaths(binDir, dll);
                        if (!File.exists(absFile))
                            continue;

                        tf.writeLine("      <Component Id='" + idFor("cmp_", dll) + "' Directory='INSTALLDIR' Guid='*'>");
                        tf.writeLine("        <File Id='" + idFor("", dll) + "' Source='" + FileInfo.toWindowsSeparators(absFile) + "' KeyPath='yes' />");
                        tf.writeLine("      </Component>");
                    }

                    tf.writeLine("    </ComponentGroup>");
                    tf.writeLine("  </Fragment>");
                    tf.writeLine("</Wix>");
                } finally {
                    if (tf)
                        tf.close();
                }
            };
            return [cmd];
        }
    }

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
