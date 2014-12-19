import qbs 1.0

DynamicLibrary {
    Depends { name: "libtiled" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui" }

    destinationDirectory: {
        if (qbs.targetOS.contains("windows"))
            return "plugins/tiled"
        else if (qbs.targetOS.contains("osx"))
            return "bin/Tiled.app/Contents/PlugIns"
        else
            return "lib/tiled/plugins"
    }
}
