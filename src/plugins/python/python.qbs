import qbs 1.0
import qbs.Probes as Probes
import qbs.File
import qbs.Environment

TiledPlugin {
    Depends { name: "Qt"; submodules: ["widgets"] }

    condition: {
        if (qbs.targetOS.contains("windows"))
            return File.exists(Environment.getEnv("PYTHONHOME"));

        return pkgConfigPython3.found ||
               pkgConfigPython3_6.found ||
               pkgConfigPython3_4.found;
    }

    Probes.PkgConfigProbe {
        id: pkgConfigPython3
        name: "python3"
    }
    Probes.PkgConfigProbe {
        id: pkgConfigPython3_6
        name: "python-3.6"
    }
    Probes.PkgConfigProbe {
        id: pkgConfigPython3_4
        name: "python-3.4"
    }

    Properties {
        condition: pkgConfigPython3.found
        cpp.cxxFlags: pkgConfigPython3.cflags
        cpp.dynamicLibraries: pkgConfigPython3.libraries
        cpp.libraryPaths: pkgConfigPython3.libraryPaths
        cpp.linkerFlags: pkgConfigPython3.linkerFlags
    }
    Properties {
        condition: !pkgConfigPython3.found && pkgConfigPython3_6.found
        cpp.cxxFlags: pkgConfigPython3_6.cflags
        cpp.dynamicLibraries: pkgConfigPython3_6.libraries
        cpp.libraryPaths: pkgConfigPython3_6.libraryPaths
        cpp.linkerFlags: pkgConfigPython3_6.linkerFlags
    }
    Properties {
        condition: !pkgConfigPython3.found && !pkgConfigPython3_6.found && pkgConfigPython3_4.found
        cpp.cxxFlags: pkgConfigPython3_4.cflags
        cpp.dynamicLibraries: pkgConfigPython3_4.libraries
        cpp.libraryPaths: pkgConfigPython3_4.libraryPaths
        cpp.linkerFlags: pkgConfigPython3_4.linkerFlags
    }

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.includePaths: [Environment.getEnv("PYTHONHOME") + "/include"]
        cpp.libraryPaths: [Environment.getEnv("PYTHONHOME") + "/libs"]
        cpp.dynamicLibraries: ["python3"]
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
