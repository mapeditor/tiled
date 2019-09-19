import qbs 1.0
import qbs.Probes as Probes
import qbs.File
import qbs.Environment
import qbs.FileInfo
import qbs.Process

DynamicLibrary {
    targetName: "tiled"

    Depends { name: "cpp" }
    Depends { name: "libtiled" }
    Depends { name: "Qt"; submodules: "gui"; versionAtLeast: "5.6" }

    cpp.dynamicLibraryPrefix: ""
    cpp.dynamicLibrarySuffix: {
        var p = new Process();
        var suffix = ".so";
        try {
            p.exec("python3-config", ["--extension-suffix"], true);
            suffix = p.readStdOut().trim();
        } finally {
            p.close();
        }
        return suffix;
    }

    condition: {
        if (qbs.targetOS.contains("windows"))
            return File.exists(Environment.getEnv("PYTHONHOME"));

        return pkgConfigPython3.found;
    }

    Probes.PkgConfigProbe {
        id: pkgConfigPython3
        name: "python3"
    }

    PythonProbe {
        id: pythonDllProbe
        pythonDir: Environment.getEnv("PYTHONHOME")
    }

    Properties {
        condition: pkgConfigPython3.found
        cpp.cxxFlags: pkgConfigPython3.cflags
        cpp.dynamicLibraries: pkgConfigPython3.libraries
        cpp.libraryPaths: pkgConfigPython3.libraryPaths
        cpp.linkerFlags: pkgConfigPython3.linkerFlags
    }

    Properties {
        condition: qbs.targetOS.contains("windows") && !qbs.toolchain.contains("mingw")
        cpp.includePaths: [Environment.getEnv("PYTHONHOME") + "/include"]
        cpp.libraryPaths: [Environment.getEnv("PYTHONHOME") + "/libs"]
        cpp.dynamicLibraries: ["python3"]
    }

    Properties {
        condition: qbs.targetOS.contains("windows") && qbs.toolchain.contains("mingw")
        cpp.includePaths: [Environment.getEnv("PYTHONHOME") + "/include"]
        cpp.libraryPaths: [Environment.getEnv("PYTHONHOME") + "/libs"]
        cpp.dynamicLibraries: [FileInfo.joinPaths(Environment.getEnv("PYTHONHOME"), pythonDllProbe.fileNamePrefix + ".dll")]
    }

    cpp.cxxLanguageVersion: "c++14"

    Properties {
        condition: qbs.targetOS.contains("darwin")
        bundle.isBundle: false
        cpp.sonamePrefix: "@rpath"
    }

    files: [
        "pytiled.cpp",
    ]

    Group {
        condition: !qbs.targetOS.contains("darwin")
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return ""
            else
                return "lib"
        }
        fileTagsFilter: "dynamiclibrary"
    }
}
