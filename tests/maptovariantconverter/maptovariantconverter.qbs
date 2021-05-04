import qbs

CppApplication {
    name: "test_maptovariantconverter"
    type: ["application", "autotest"]

    Depends { name: "libtiled" }
    Depends { name: "Qt.testlib" }

    cpp.cxxLanguageVersion: "c++14"

    files: [
        "test_maptovariantconverter.cpp",
    ]
}
