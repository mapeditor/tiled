import qbs

CppApplication {
    name: "test_mapreader"
    type: ["application", "autotest"]

    Depends { name: "libtiled" }
    Depends { name: "Qt.testlib" }

    cpp.cxxLanguageVersion: "c++14"

    files: [
        "mapreader.qrc",
        "test_mapreader.cpp",
    ]
}
