import qbs 1.0

StaticLibrary {
    name: "qtsingleapplication"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["widgets", "network"] }

    cpp.includePaths: ["src"]
    cpp.cxxLanguageVersion: "c++11"

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
