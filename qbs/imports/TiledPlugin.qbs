import qbs 1.0

DynamicLibrary {
    Depends { name: "libtiled" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui" }

    cpp.cxxLanguageVersion: "c++11"
    cpp.visibility: "minimal"
    bundle.isBundle: false

    Properties {
        condition: qbs.targetOS.contains("osx")
        cpp.cxxFlags: ["-Wno-unknown-pragmas"]
    }

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows") || project.linuxArchive)
                return "plugins/tiled"
            else if (qbs.targetOS.contains("osx"))
                return "Tiled.app/Contents/PlugIns"
            else
                return "lib/tiled/plugins"
        }
        fileTagsFilter: "dynamiclibrary"
    }

    FileTagger {
        patterns: "plugin.json"
        fileTags: ["qt_plugin_metadata"]
    }
}
