import qbs 1.0

StaticLibrary {
    name: "qtsingleapplication"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["widgets", "network"]; versionAtLeast: "5.4" }

    cpp.includePaths: ["src"]
    cpp.cxxLanguageVersion: "c++14"

    files: [
        "src/qtlocalpeer.cpp",
        "src/qtlocalpeer.h",
        "src/qtsingleapplication.cpp",
        "src/qtsingleapplication.h",
    ]

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt.network" }
        cpp.includePaths: "src"
    }
}
