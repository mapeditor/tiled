TiledTest {
    name: "test_export"

    Depends { name: "libtilededitor" }

    cpp.includePaths: [
        "../../src/tiled",
    ]

    files: [
        "test_export.cpp",
    ]
}
