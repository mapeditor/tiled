import qbs

CppApplication {
    name: "test_staggeredrenderer"
    type: ["application", "autotest"]

    Depends { name: "libtiled" }
    Depends { name: "Qt.testlib" }

    cpp.cxxLanguageVersion: "c++14"

    files: [
        "test_staggeredrenderer.cpp",
    ]
}
