import qbs 1.0
import qbs.Probes as Probes
import qbs.File
import qbs.Environment

TiledPlugin {
    Depends { name: "Qt"; submodules: ["widgets"] }

    condition: {
        if (qbs.targetOS.contains("windows")) {
            return File.exists( Environment.getEnv("PYTHONHOME") );
        }

        return pkgConfig.found;
    }

    Probes.PkgConfigProbe {
        id: pkgConfig
        // the default qbs.sysroot basically just messes up system defaults
        sysroot: ''
        packageNames: ["python3"]
    }

    Properties {
        condition: pkgConfig.found

        cpp.cxxFlags: pkgConfig.cflags
        cpp.linkerFlags: pkgConfig.libs
        // was ldflags broken for some py2 pkgconfigs? seems to work with py3
        //cpp.dynamicLibraries: ["python3"]
    }

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.includePaths: [Environment.getEnv("PYTHONHOME")+"/include"]
        cpp.libraryPaths: [Environment.getEnv("PYTHONHOME")+"/libs"]
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
