import qbs 1.0
import qbs.Probes as Probes
import qbs.File

TiledPlugin {
    Depends { name: "Qt"; submodules: ["widgets"] }

    condition: {
        if (qbs.targetOS.contains("linux")) {
            return true;
        } else if (qbs.targetOS.contains("windows")) {
            // On Windows, currently only the default install location of
            // Python 2.7 is supported, and only when compiling with MinGW in
            // release mode. Also, it needs to be Python 2.7.9, since 2.7.10
            // results in a linker error.
            //
            // https://www.python.org/ftp/python/2.7.9/python-2.7.9.msi
            //
            return File.exists("C:/Python27") &&
                    qbs.toolchain.contains("mingw") &&
                    !qbs.debugInformation;
        }

        // Not sure how to properly support Python on Mac OS X yet
        // (possibly requires using python-config)
        return false;
    }

    Probes.PkgConfigProbe {
        id: pkgConfig
        name: "python-2.7"
    }

    Properties {
        condition: qbs.targetOS.contains("linux")

        cpp.cxxFlags: pkgConfig.cflags
        // This should be it, but it doesn't work because the -lpython2.7 ends
        // up too early on the command line.
        //cpp.linkerFlags: pkgConfig.libs
        cpp.dynamicLibraries: ["python2.7"]
    }

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.includePaths: ["C:/Python27/include"]
        cpp.libraryPaths: ["C:/Python27/libs"]
        cpp.dynamicLibraries: ["python27"]
    }

    files: [
        "plugin.json",
        "pythonplugin.cpp",
        "pythonplugin.h",
        "pythonbind.cpp",
    ]
}
