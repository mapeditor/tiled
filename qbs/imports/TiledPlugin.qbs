import qbs 1.0

DynamicLibrary {
    Depends { name: "libtiled" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui" }

    cpp.cxxLanguageVersion: "c++11"

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows"))
                return "plugins/tiled"
            else if (qbs.targetOS.contains("osx"))
                return "Tiled.app/Contents/PlugIns"
            else
                return "lib/tiled/plugins"
        }
        fileTagsFilter: "dynamiclibrary"
    }
}
