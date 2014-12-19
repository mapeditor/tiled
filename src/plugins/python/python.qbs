import qbs 1.0
import qbs.Probes as Probes

TiledPlugin {
    Depends { name: "Qt"; submodules: ["widgets"] }

    Probes.PkgConfigProbe {
        id: pkgConfig
        name: "python-2.7"
    }

    cpp.cxxFlags: pkgConfig.cflags
    // This should be it, but it doesn't work because the -lpython2.7 ends up
    // too early on the command line.
    //cpp.linkerFlags: pkgConfig.libs
    cpp.dynamicLibraries: ["python2.7"]

    // TODO: Figure out why this doesn't work or another way to do it
    //condition: pkgConfig.cflags != undefined

    files: [
        "pythonplugin.cpp",
        "pythonplugin.h",
        "pythonbind.cpp",
    ]
}
