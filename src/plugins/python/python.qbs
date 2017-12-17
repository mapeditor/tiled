import qbs 1.0
import qbs.Probes as Probes
import qbs.File

TiledPlugin {
    Depends { name: "Qt"; submodules: ["widgets"] }

    condition: {
        if (qbs.targetOS.contains("linux")) {
            return pkgConfig.found;
        } else if (qbs.targetOS.contains("windows")) {
            // On Windows, currently only the default install location of
            // Python 2.7 is supported, and only when compiling with MinGW in
            // release mode. Also, avoid Python 2.7.10, since it results in a
            // linker error.
            //
            return File.exists("C:/Python27") &&
                    qbs.toolchain.contains("mingw") &&
                    !qbs.debugInformation;
        } else if (qbs.targetOS.contains("darwin")) {
            return true;
        }

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

    Properties {
        condition: qbs.targetOS.contains("darwin")
        cpp.includePaths: ["/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7"]
        cpp.libraryPaths: ["/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config"]
        cpp.dynamicLibraries: ["python2.7"]
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
