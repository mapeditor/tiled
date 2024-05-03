import qbs.Probes as Probes
import qbs.File
import qbs.Environment
import qbs.FileInfo

TiledPlugin {
    Depends { name: "Qt"; submodules: ["widgets"] }

    condition: {
        if (qbs.targetOS.contains("windows"))
            return pythonDllProbe.found;

        return pkgConfigPython3.found;
    }

    cpp.cxxFlags: {
        var flags = base
        if (qbs.toolchain.contains("gcc") && !qbs.toolchain.contains("clang"))
            flags.push("-Wno-cast-function-type")
        return flags
    }

    Probes.PkgConfigProbe {
        id: pkgConfigPython3
        name: "python3-embed"
        minVersion: "3.8"
    }

    PythonProbe {
        id: pythonDllProbe
        pythonDir: Environment.getEnv("PYTHONHOME")
        minVersion: "3.8"
    }

    Properties {
        condition: pkgConfigPython3.found
        cpp.cxxFlags: outer.concat(pkgConfigPython3.cflags)
        cpp.dynamicLibraries: pkgConfigPython3.libraries
        cpp.libraryPaths: pkgConfigPython3.libraryPaths
        cpp.linkerFlags: pkgConfigPython3.linkerFlags
    }

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.includePaths: [Environment.getEnv("PYTHONHOME") + "/include"]
        cpp.libraryPaths: [Environment.getEnv("PYTHONHOME") + "/libs"]
        cpp.dynamicLibraries: {
            if (qbs.toolchain.contains("mingw"))
                return [FileInfo.joinPaths(Environment.getEnv("PYTHONHOME"), pythonDllProbe.fileNamePrefix + ".dll")];
            return ["python3"];
        }
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
