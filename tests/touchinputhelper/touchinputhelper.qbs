TiledTest {
    name: "test_touchinputhelper"

    Depends { name: "libtilededitor" }
    Depends { name: "Qt.widgets" }

    files: [
        "test_touchinputhelper.cpp",
    ]
}
