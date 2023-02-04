import qbs 1.0

DynamicLibrary {
    Depends { name: "libtiled" }
    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: "gui" }

    cpp.cxxLanguageVersion: "c++17"
    cpp.cxxFlags: {
        var flags = base;
        if (qbs.toolchain.contains("msvc")) {
            if (Qt.core.versionMajor >= 6 && Qt.core.versionMinor >= 3)
                flags.push("/permissive-");
        }
        return flags;
    }
    cpp.visibility: "minimal"
    cpp.useRPaths: project.useRPaths
    cpp.rpaths: {
        if (qbs.targetOS.contains("darwin"))
            return ["@loader_path/../Frameworks"];
        else
            return ["$ORIGIN/../.."];
    }
    cpp.defines: [
        "QT_DISABLE_DEPRECATED_BEFORE=QT_VERSION_CHECK(5,15,0)",
        "QT_NO_DEPRECATED_WARNINGS",
        "QT_NO_FOREACH",
        "QT_NO_URL_CAST_FROM_STRING"
    ]

    Properties {
        condition: qbs.targetOS.contains("macos")
        bundle.isBundle: false
    }

    Group {
        qbs.install: true
        qbs.installDir: {
            if (project.windowsLayout)
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
