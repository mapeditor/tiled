import qbs 1.0

DynamicLibrary {
    Depends { name: "libtiled" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui" }

    cpp.cxxLanguageVersion: "c++11"
    cpp.visibility: "minimal"
    cpp.useRPaths: project.useRPaths
    bundle.isBundle: false

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.cxxFlags: ["-Wno-unknown-pragmas"]
    }

    Group {
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows") || project.linuxArchive)
                return "plugins/tiled"
            else if (qbs.targetOS.contains("macos"))
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
