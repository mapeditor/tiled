TiledTest {
    name: "test_mcpserver"

    Depends { name: "libtilededitor" }
    Depends { name: "Qt.httpserver"; versionAtLeast: "6.4"; required: false }
    condition: Qt.httpserver.present

    files: [
        "test_mcpserver.cpp",
    ]
}
