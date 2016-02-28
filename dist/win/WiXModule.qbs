/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

import qbs
import qbs.File
import qbs.FileInfo
import qbs.ModUtils

Module {
    condition: qbs.targetOS.contains("windows")

    property path toolchainInstallPath: qbs.getNativeSetting(registryKey, "InstallFolder")
    property path toolchainInstallRoot: qbs.getNativeSetting(registryKey, "InstallRoot")
    property string version: qbs.getNativeSetting(registryKey, "ProductVersion")
    property var versionParts: {
        var parts = version ? version.split('.').map(function(item) { return parseInt(item, 10); }) : []
        if (parts.length == 3) {
            parts.push(parts[2]);
            parts[2] = 0;
        }
        return parts;
    }
    property int versionMajor: versionParts[0]
    property int versionMinor: versionParts[1]
    property int versionPatch: versionParts[2]
    property int versionBuild: versionParts[3]

    property string compilerName: "candle.exe"
    property string compilerPath: FileInfo.joinPaths(toolchainInstallRoot, compilerName)
    property string linkerName: "light.exe"
    property string linkerPath: FileInfo.joinPaths(toolchainInstallRoot, linkerName)

    property string warningLevel: "normal"
    PropertyOptions {
        name: "warningLevel"
        allowedValues: ["none", "normal", "pedantic"]
    }

    property bool debugInformation: qbs.debugInformation
    property bool treatWarningsAsErrors: false
    property bool verboseOutput: false
    PropertyOptions {
        name: "verboseOutput"
        description: "display verbose output from the compiler and linker"
    }

    property bool visualStudioCompatibility: true
    PropertyOptions {
        name: "visualStudioCompatibility"
        description: "whether to define most of the same variables as " +
                     "Visual Studio when using the Candle compiler"
    }

    property bool enableQbsDefines: true
    PropertyOptions {
        name: "enableQbsDefines"
        description: "built-in variables that are defined when using the Candle compiler"
    }

    property pathList includePaths
    PropertyOptions {
        name: "includePaths"
        description: "directories to add to the include search path"
    }

    property stringList defines
    PropertyOptions {
        name: "defines"
        description: "variables that are defined when using the Candle compiler"
    }

    property stringList compilerFlags
    PropertyOptions {
        name: "compilerFlags"
        description: "additional flags for the Candle compiler"
    }

    property stringList linkerFlags
    PropertyOptions {
        name: "linkerFlags"
        description: "additional flags for the Light linker"
    }

    property stringList cultures
    PropertyOptions {
        name: "cultures"
        description: "the list of localizations to build the MSI for; leave undefined to build all localizations"
    }

    property stringList extensions: product.type.contains("wixsetup") ? ["WixBalExtension"] : [] // default to WiX Standard Bootstrapper extension

    // private properties
    property string targetSuffix: {
        if (product.type.contains("msi")) {
            return windowsInstallerSuffix;
        } else if (product.type.contains("wixsetup")) {
            return executableSuffix;
        }
    }

    // MSI/MSM package validation only works natively on Windows
    property bool enablePackageValidation: qbs.hostOS.contains("windows")

    property string executableSuffix: ".exe"
    property string windowsInstallerSuffix: ".msi"

    property string registryKey: {
        var knownVersions = [ "4.0", "3.10", "3.9", "3.8", "3.7", "3.6", "3.5", "3.0", "2.0" ];
        var keyNative = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Installer XML\\";
        var keyWoW64 = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows Installer XML\\";

        for (i in knownVersions) {
            if (qbs.getNativeSetting(keyNative + knownVersions[i], "ProductVersion"))
                return keyNative + knownVersions[i];
            if (qbs.getNativeSetting(keyWoW64 + knownVersions[i], "ProductVersion"))
                return keyWoW64 + knownVersions[i];
        }
    }

    validate: {
        var validator = new ModUtils.PropertyValidator("wix");
        validator.setRequiredProperty("toolchainInstallPath", toolchainInstallPath);
        validator.setRequiredProperty("toolchainInstallRoot", toolchainInstallRoot);
        validator.setRequiredProperty("version", version);
        validator.setRequiredProperty("versionMajor", versionMajor);
        validator.setRequiredProperty("versionMinor", versionMinor);
        validator.setRequiredProperty("versionPatch", versionPatch);
        validator.setRequiredProperty("versionBuild", versionBuild);
        validator.addVersionValidator("version", version, 3, 4);
        validator.addRangeValidator("versionMajor", versionMajor, 1);
        validator.addRangeValidator("versionMinor", versionMinor, 0);
        validator.addRangeValidator("versionPatch", versionPatch, 0);
        validator.addRangeValidator("versionBuild", versionBuild, 0);
        validator.validate();
    }

    setupBuildEnvironment: {
        var v = new ModUtils.EnvironmentVariable("PATH", qbs.pathListSeparator, true);
        v.prepend(toolchainInstallPath);
        v.prepend(toolchainInstallRoot);
        v.set();
    }

    // WiX Include Files
    FileTagger {
        patterns: ["*.wxi"]
        fileTags: ["wxi"]
    }

    // WiX Localization Files
    FileTagger {
        patterns: ["*.wxl"]
        fileTags: ["wxl"]
    }

    // WiX Source Files
    FileTagger {
        patterns: ["*.wxs"]
        fileTags: ["wxs"]
    }

    Rule {
        id: candleCompiler
        inputs: ["wxs"]
        auxiliaryInputs: ['wxi']

        Artifact {
            fileTags: ["wixobj"]
            filePath: FileInfo.joinPaths(".obj", qbs.getHash(input.baseDir),
                                         FileInfo.baseName(input.fileName) + ".wixobj")
        }

        prepare: {
            var i;
            var args = ["-nologo"];

            if (ModUtils.moduleProperty(input, "warningLevel") === "none") {
                args.push("-sw");
            } else {
                if (ModUtils.moduleProperty(input, "warningLevel") === "pedantic") {
                    args.push("-pedantic");
                }

                if (ModUtils.moduleProperty(input, "treatWarningsAsErrors")) {
                    args.push("-wx");
                }
            }

            if (ModUtils.moduleProperty(input, "verboseOutput")) {
                args.push("-v");
            }

            var arch = product.moduleProperty("qbs", "architecture");
            if (!["x86", "x86_64", "ia64", "arm"].contains(arch)) {
                // http://wixtoolset.org/documentation/manual/v3/xsd/wix/package.html
                throw("WiX: unsupported architecture '" + arch + "'");
            }

            // QBS uses "x86_64", Microsoft uses "x64"
            if (arch === "x86_64") {
                arch = "x64";
            }

            // Visual Studio defines these variables along with various solution and project names and paths;
            // we'll pass most of them to ease compatibility between QBS and WiX projects originally created
            // using Visual Studio. The only definitions we don't pass are the ones which make no sense at all
            // in QBS, like the solution and project directories since they do not exist.
            if (ModUtils.moduleProperty(input, "visualStudioCompatibility")) {
                var toolchain = product.moduleProperties("qbs", "toolchain");
                var toolchainInstallPath = product.moduleProperty("cpp", "toolchainInstallPath");
                if (toolchain && toolchain.contains("msvc") && toolchainInstallPath) {
                    var vcDir = toolchainInstallPath.replace(/[\\/]bin$/i, "");
                    var vcRootDir = vcDir.replace(/[\\/]VC$/i, "");
                    args.push("-dDevEnvDir=" + FileInfo.toWindowsSeparators(FileInfo.joinPaths(vcRootDir, 'Common7', 'IDE')));
                }

                var buildVariant = product.moduleProperty("qbs", "buildVariant");
                if (buildVariant === "debug") {
                    args.push("-dDebug");
                    args.push("-dConfiguration=Debug");
                } else if (buildVariant === "release") {
                    // VS doesn't define "Release"
                    args.push("-dConfiguration=Release");
                }

                var productTargetExt = ModUtils.moduleProperty(input, "targetSuffix");
                if (!productTargetExt) {
                    throw("WiX: Unsupported product type '" + product.type + "'");
                }

                var builtBinaryFilePath = FileInfo.joinPaths(product.buildDirectory, product.destinationDirectory, product.targetName + productTargetExt);
                args.push("-dOutDir=" + FileInfo.toWindowsSeparators(FileInfo.path(builtBinaryFilePath))); // in VS, relative to the project file by default

                args.push("-dPlatform=" + arch);

                args.push("-dProjectName=" + project.name);

                args.push("-dTargetDir=" + FileInfo.toWindowsSeparators(FileInfo.path(builtBinaryFilePath))); // in VS, an absolute path
                args.push("-dTargetExt=" + productTargetExt);
                args.push("-dTargetFileName=" + product.targetName + productTargetExt);
                args.push("-dTargetName=" + product.targetName);
                args.push("-dTargetPath=" + FileInfo.toWindowsSeparators(builtBinaryFilePath));
            }

            var includePaths = ModUtils.moduleProperties(input, "includePaths");
            for (i in includePaths) {
                args.push("-I" + includePaths[i]);
            }

            var enableQbsDefines = ModUtils.moduleProperty(input, "enableQbsDefines")
            if (enableQbsDefines) {
                var map = {
                    "project.": project,
                    "product.": product
                };

                for (var prefix in map) {
                    var obj = map[prefix];
                    for (var prop in obj) {
                        var val = obj[prop];
                        if (typeof val !== 'function' && typeof val !== 'object' && !prop.startsWith("_")) {
                            args.push("-d" + prefix + prop + "=" + val);
                        }
                    }
                }

                // Helper define for alternating between 32-bit and 64-bit logic
                if (arch === "x64" || arch === "ia64") {
                    args.push("-dWin64");
                }
            }

            // User-supplied defines
            var defines = ModUtils.moduleProperties(input, "defines");
            for (i in defines) {
                args.push("-d" + defines[i]);
            }

            // User-supplied flags
            var flags = ModUtils.moduleProperties(input, "compilerFlags");
            for (i in flags) {
                args.push(flags[i]);
            }

            args.push("-out");
            args.push(FileInfo.toWindowsSeparators(output.filePath));
            args.push("-arch");
            args.push(arch);

            var extensions = ModUtils.moduleProperties(input, "extensions");
            for (i in extensions) {
                args.push("-ext");
                args.push(extensions[i]);
            }

            args.push(FileInfo.toWindowsSeparators(input.filePath));

            var cmd = new Command(ModUtils.moduleProperty(product, "compilerPath"), args);
            cmd.description = "compiling " + input.fileName;
            cmd.highlight = "compiler";
            cmd.workingDirectory = FileInfo.path(output.filePath);
            return cmd;
        }
    }

    Rule {
        id: lightLinker
        multiplex: true
        inputs: ["wixobj", "wxl"]

        outputArtifacts: {
            var artifacts = [];

            if (product.type.contains("wixsetup")) {
                artifacts.push({
                    fileTags: ["wixsetup", "application"],
                    filePath: FileInfo.joinPaths(product.destinationDirectory,
                                                 product.targetName
                                                    + ModUtils.moduleProperty(product,
                                                                          "executableSuffix"))
                });
            }

            if (product.type.contains("msi")) {
                artifacts.push({
                    fileTags: ["msi"],
                    filePath: FileInfo.joinPaths(product.destinationDirectory,
                                                 product.targetName
                                                    + ModUtils.moduleProperty(product,
                                                                          "windowsInstallerSuffix"))
                });
            }

            if (ModUtils.moduleProperty(product, "debugInformation")) {
                artifacts.push({
                    fileTags: ["wixpdb"],
                    filePath: FileInfo.joinPaths(product.destinationDirectory,
                                                 product.targetName + ".wixpdb")
                });
            }

            return artifacts;
        }

        outputFileTags: ["application", "msi", "wixpdb", "wixsetup"]

        prepare: {
            var i;
            var primaryOutput;
            if (product.type.contains("wixsetup")) {
                primaryOutput = outputs.wixsetup[0];
            } else if (product.type.contains("msi")) {
                primaryOutput = outputs.msi[0];
            } else {
                throw("WiX: Unsupported product type '" + product.type + "'");
            }

            var args = ["-nologo"];

            if (!ModUtils.moduleProperty(product, "enablePackageValidation")) {
                args.push("-sval");
            }

            if (ModUtils.moduleProperty(product, "warningLevel") === "none") {
                args.push("-sw");
            } else {
                if (ModUtils.moduleProperty(product, "warningLevel") === "pedantic") {
                    args.push("-pedantic");
                }

                if (ModUtils.moduleProperty(product, "treatWarningsAsErrors")) {
                    args.push("-wx");
                }
            }

            if (ModUtils.moduleProperty(product, "verboseOutput")) {
                args.push("-v");
            }

            args.push("-out");
            args.push(FileInfo.toWindowsSeparators(primaryOutput.filePath));

            if (ModUtils.moduleProperty(product, "debugInformation")) {
                args.push("-pdbout");
                args.push(FileInfo.toWindowsSeparators(outputs.wixpdb[0].filePath));
            } else {
                args.push("-spdb");
            }

            var extensions = ModUtils.moduleProperties(product, "extensions");
            for (i in extensions) {
                args.push("-ext");
                args.push(extensions[i]);
            }

            for (i in inputs.wxl) {
                args.push("-loc");
                args.push(FileInfo.toWindowsSeparators(inputs.wxl[i].filePath));
            }

            if (product.type.contains("msi")) {
                var cultures = ModUtils.moduleProperties(product, "cultures");
                args.push("-cultures:"
                    + (cultures && cultures.length > 0 ? cultures.join(";") : "null"));
            }

            // User-supplied flags
            var flags = ModUtils.moduleProperties(product, "linkerFlags");
            for (i in flags) {
                args.push(flags[i]);
            }

            for (i in inputs.wixobj) {
                args.push(FileInfo.toWindowsSeparators(inputs.wixobj[i].filePath));
            }

            var cmd = new Command(ModUtils.moduleProperty(product, "linkerPath"), args);
            cmd.description = "linking " + primaryOutput.fileName;
            cmd.highlight = "linker";
            cmd.workingDirectory = FileInfo.path(primaryOutput.filePath);
            return cmd;
        }
    }
}
