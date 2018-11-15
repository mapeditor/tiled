import qbs 1.0
import qbs.Probes as Probes
import qbs.File
import qbs.Environment
import qbs.FileInfo

TiledPlugin {
    Depends { name: "Qt"; submodules: ["widgets"] }

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

    files: [
        "plugin.json",
        "pythonplugin.cpp",
        "pythonplugin.h",
        "pythonbind.cpp",
        "qtbinding.py",
        "tiledbinding.py",
    ]
}
