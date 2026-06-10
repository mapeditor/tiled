import qbs.FileInfo

StaticLibrary {
    name: "qtsingleapplication"

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["widgets", "network"]; versionAtLeast: "5.4" }

    cpp.includePaths: ["src"]
    cpp.cxxLanguageVersion: "c++17"
    cpp.visibility: "minimal"

    // Avoid wrapping this in a static framework, whose code signing breaks
    // the install step on macOS 26 with Qbs 3.2.0.
    Properties {
        condition: qbs.targetOS.contains("macos")
        bundle.isBundle: false
    }

    files: [
        "src/qtlocalpeer.cpp",
        "src/qtlocalpeer.h",
        "src/qtsingleapplication.cpp",
        "src/qtsingleapplication.h",
    ]

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt.widgets" }
        cpp.includePaths: FileInfo.joinPaths(exportingProduct.sourceDirectory, "src")
    }
}
