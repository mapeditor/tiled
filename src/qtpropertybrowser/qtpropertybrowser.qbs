import qbs 1.0

StaticLibrary {
    name: "qtpropertybrowser"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["widgets"] }

    cpp.includePaths: ["src"]
    cpp.cxxLanguageVersion: "c++14"

    Properties {
        // needed to work around "too many sections" issue in qteditorfactory.cpp
        condition: Qt.core.versionMajor >= 6 && Qt.core.versionMinor >= 2 && qbs.toolchain.contains("mingw")
        cpp.cxxFlags: ["-Wa,-mbig-obj"]
    }

    files: [
        "src/qtbuttonpropertybrowser.cpp",
        "src/qtbuttonpropertybrowser.h",
        "src/qteditorfactory.cpp",
        "src/qteditorfactory.h",
        "src/qtgroupboxpropertybrowser.cpp",
        "src/qtgroupboxpropertybrowser.h",
        "src/qtpropertybrowser.cpp",
        "src/qtpropertybrowser.h",
        "src/qtpropertybrowserutils.cpp",
        "src/qtpropertybrowserutils_p.h",
        "src/qtpropertymanager.cpp",
        "src/qtpropertymanager.h",
        "src/qttreepropertybrowser.cpp",
        "src/qttreepropertybrowser.h",
        "src/qtvariantproperty.cpp",
        "src/qtvariantproperty.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: "src"
    }
}
