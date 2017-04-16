import qbs

CppApplication {
    name: "tst_quicktiledplugin"
    type: ["application", "autotest"]
    targetName: name

    Depends { name: "cpp" }
    Depends { name: "Qt"; submodules: ["core", "gui", "test", "quick"] }

    files: [
        "tst_quicktiledplugin.cpp"
    ]
}
