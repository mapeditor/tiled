/*
  Installs all files necessary to run Tiled and other files that should be
  shipped when Tiled is distributed.
*/

import qbs.File
import qbs.FileInfo

Product {
    name: "distribute"
    type: "installable"
    builtByDefault: (project.snapshot || project.release) && qbs.targetOS.contains("windows")

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }

    Group {
        name: "Examples"
        prefix: "../examples/"
        files: ["**"]

        qbs.install: true
        qbs.installDir: "examples"
        qbs.installSourceBase: prefix
    }

    Group {
        name: "Python Scripts"
        prefix: "../src/plugins/python/scripts/"
        files: ["**"]

        qbs.install: true
        qbs.installDir: "examples/python"
        qbs.installSourceBase: prefix
    }

    Group {
        name: "Qt DLLs"
        prefix: {
            if (qbs.targetOS.contains("windows")) {
                return Qt.core.binPath + "/"
            } else {
                return Qt.core.libPath + "/lib"
            }
        }
        property string postfix: {
            var suffix = "";
            if (qbs.targetOS.contains("windows") && qbs.debugInformation && Qt.core.versionMajor < 6 && Qt.core.versionMinor < 15)
                suffix += "d";
            return suffix + cpp.dynamicLibrarySuffix;
        }
        files: {
            function addQtVersions(libs) {
                var result = [];
                for (i = 0; i < libs.length; ++i) {
                    var lib = libs[i]
                    var major = lib + "." + Qt.core.versionMajor;
                    var minor = major + "." + Qt.core.versionMinor;
                    var patch = minor + "." + Qt.core.versionPatch;
                    if (File.exists(minor))
                        result.push(minor)
                    if (File.exists(lib))
                        result.push(lib)
                    result.push(major, patch);
                }
                return result;
            }

            var list = [];

            if (!Qt.core.frameworkBuild) {
                var major = Qt.core.versionMajor;
                list.push(
                    "Qt" + major + "Concurrent" + postfix,
                    "Qt" + major + "Core" + postfix,
                    "Qt" + major + "Gui" + postfix,
                    "Qt" + major + "Network" + postfix,
                    "Qt" + major + "Qml" + postfix,
                    "Qt" + major + "Svg" + postfix,
                    "Qt" + major + "Widgets" + postfix
                );

                if (major >= 6) {
                    list.push(
                        "Qt" + major + "OpenGL" + postfix,
                        "Qt" + major + "OpenGLWidgets" + postfix
                    );
                }
            }

            if (qbs.targetOS.contains("linux")) {
                list = addQtVersions(list);
                list = list.concat(addQtVersions([
                    "Qt" + major + "DBus.so",
                    "Qt" + major + "XcbQpa.so",
                ]))

                if (File.exists(prefix + "icudata.so.56")) {
                    list.push("icudata.so.56", "icudata.so.56.1");
                    list.push("icui18n.so.56", "icui18n.so.56.1");
                    list.push("icuuc.so.56", "icuuc.so.56.1");
                }
            }

            return list;
        }
        qbs.install: true
        qbs.installDir: qbs.targetOS.contains("windows") ? "" : project.libDir
    }

    property var pluginFiles: {
        if (qbs.targetOS.contains("windows")) {
            if (qbs.debugInformation)
                return ["*d.dll"];
            else
                return ["*.dll"];
        } else if (qbs.targetOS.contains("linux")) {
            return ["*.so"];
        }
        return ["*"];
    }

    property var pluginExcludeFiles: {
        var files = ["*.pdb"];
        if (!(qbs.targetOS.contains("windows") && qbs.debugInformation)) {
            // Exclude debug DLLs.
            //
            // This also excludes the qdirect2d.dll platform plugin, but I'm
            // not sure when it would be preferable over the qwindows.dll. In
            // testing it, it seems to have severe issues with HiDpi screens
            // (as of Qt 5.8.0).
            files.push("*d.dll");
        }
        return files;
    }

    Group {
        name: "Qt Icon Engine Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/iconengines/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: "plugins/iconengines"
    }

    Group {
        name: "Qt Image Format Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/imageformats/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: "plugins/imageformats"
    }

    Group {
        name: "Qt Platform Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/platforms/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: "plugins/platforms"
    }

    Group {
        name: "Qt Platform Input Context Plugins"
        condition: qbs.targetOS.contains("linux")
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/platforminputcontexts/")
        files: pluginFiles
        qbs.install: true
        qbs.installDir: "plugins/platforminputcontexts"
    }

    Group {
        name: "Qt Platform Theme Plugins"
        condition: qbs.targetOS.contains("linux")
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/platformthemes/")
        files: pluginFiles
        qbs.install: true
        qbs.installDir: "plugins/platformthemes"
    }

    Group {
        name: "Qt Style Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/styles/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: "plugins/styles"
    }

    Group {
        name: "Qt TLS Plugins"
        condition: Qt.core.versionMajor >= 6 && Qt.core.versionMinor >= 2;
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/tls/")
        files: {
            if (qbs.targetOS.contains("windows")) {
                if (qbs.debugInformation)
                    return ["qschannelbackendd.dll"];
                else
                    return ["qschannelbackend.dll"];
            } else if (qbs.targetOS.contains("macos")) {
                return ["libqsecuretransportbackend.dylib"];
            }

            return pluginFiles;
        }
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: "plugins/tls"
    }

    Group {
        name: "Qt XCB GL Integration Plugins"
        condition: qbs.targetOS.contains("linux")
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/xcbglintegrations/")
        files: pluginFiles
        qbs.install: true
        qbs.installDir: "plugins/xcbglintegrations"
    }

    Group {
        name: "Qt Translations"
        prefix: {
            if (qbs.targetOS.contains("windows")) {
                return FileInfo.joinPaths(Qt.core.binPath, "../translations/")
            } else if (qbs.targetOS.contains("linux")) {
                return FileInfo.joinPaths(Qt.core.libPath, "../translations/")
            }
        }
        files: {
            // Since Qt 5.5, the translations are split up by module and failing
            // to include all the .qm files results in the loading of the Qt
            // translator to fail (regardless of whether these modules are
            // actually used).
            var modules = ["",
                           "base",
                           "multimedia",
                           "quick1",
                           "script",
                           "xmlpatterns"];

            // TODO: Look into getting this list from the Tiled
            // 'translations.qbs' product.
            var languages = ["ar_DZ",
                             "bg",
                             "cs",
                             "de",
                             "en",
                             "es",
                             "fr",
                             "he",
                             "it",
                             "ja",
                             "lv",
                             "nb",
                             "nl",
                             "pl",
                             "pt",
                             "pt_PT",
                             "ru",
                             "sv",
                             "th",
                             "tr",
                             "uk",
                             "zh_CN",
                             "zh_TW"];

            var list = [];
            var p = prefix;

            for (i = 0; i < languages.length; ++i) {
                for (j = 0; j < modules.length; ++j) {
                    var file = "qt" + modules[j] + "_" + languages[i] + ".qm";
                    if (File.exists(p + file))
                        list.push(file);
                }
            }

            return list;
        }
        qbs.install: true
        qbs.installDir: "translations"
    }

    Group {
        name: "Runtime DLLs"
        condition: qbs.targetOS.contains("windows")

        prefix: {
            if (qbs.toolchain.contains("mingw"))
                return FileInfo.joinPaths(cpp.toolchainInstallPath) + "/"
            else if (qbs.architecture === "x86_64")
                return "C:/windows/system32/"
            else
                return "C:/windows/SysWOW64/"
        }
        files: {
            var list = []
            if (qbs.toolchain.contains("mingw")) {
                list.push("libstdc++-6.dll",
                          "libwinpthread-1.dll")

                if (qbs.architecture == "x86_64")
                    list.push("libgcc_s_seh-1.dll")
                else
                    list.push("libgcc_s_dw2-1.dll")
            } else {
                list.push("MSVCP120.DLL",
                          "MSVCR120.DLL")
            }
            return list
        }
        qbs.install: true
        qbs.installDir: ""
    }

    Group {
        name: "OpenSSL DLLs"
        condition: {
            return qbs.targetOS.contains("windows") &&
                    !(Qt.core.versionMajor >= 6 && Qt.core.versionMinor >= 2) &&
                    File.exists(prefix)
        }

        prefix: {
            if (project.openSslPath) {
                return project.openSslPath + "/";
            } else {
                if (qbs.architecture === "x86_64")
                    return "C:/OpenSSL-v111-Win64/"
                else
                    return "C:/OpenSSL-v111-Win32/"
            }
        }
        files: {
            if (qbs.architecture === "x86_64")
                return [ "libcrypto-1_1-x64.dll", "libssl-1_1-x64.dll" ]
            else
                return [ "libcrypto-1_1.dll", "libssl-1_1.dll" ]
        }
        qbs.install: true
        qbs.installDir: ""
    }

    Group {
        name: "Misc Files"
        prefix: "../"
        files: {
            var list = [
                "COPYING",
                "AUTHORS",
                "README.md",
                "NEWS.md",
                "LICENSE.APACHE",
                "LICENSE.BSD",
                "LICENSE.GPL",
            ];

            if (qbs.targetOS.contains("windows"))
                list.push("dist/win/qt.conf");
            else if (qbs.targetOS.contains("linux"))
                list.push("dist/linux/qt.conf");

            return list;
        }
        qbs.install: true
        qbs.installDir: ""
    }
}
