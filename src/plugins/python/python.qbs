import qbs 1.0
import qbs.Probes as Probes

TiledPlugin {
    Depends { name: "Qt"; submodules: ["widgets"] }

    Properties {
        condition: qbs.targetOS.contains("linux")

        Probes.PkgConfigProbe {
            id: pkgConfig
            name: "python-2.7"
        }

        cpp.cxxFlags: pkgConfig.cflags
        // This should be it, but it doesn't work because the -lpython2.7 ends
        // up too early on the command line.
        //cpp.linkerFlags: pkgConfig.libs
        cpp.dynamicLibraries: ["python2.7"]
    }

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.cxxFlags: "-IC:/Python27/include"
        cpp.linkerFlags: "-LC:/Python27/libs"
        cpp.dynamicLibraries: ["python27"]
    }

    files: [
        "pythonplugin.cpp",
        "pythonplugin.h",
        "pythonbind.cpp",
    ]
}
