TiledTest {
    name: "test_export"

    Depends { name: "libtilededitor" }

    cpp.includePaths: [
        "/home/leenattress/Projects/tiled/src/tiled",
    ]

    files: [
        "test_export.cpp",
    ]
}
