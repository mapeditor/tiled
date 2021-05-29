import qbs

Project {
    name: "tests"

    references: [
        "mapreader",
        "maptovariantconverter",
        "object",
        "staggeredrenderer",
        "varianttomapconverter"
    ]
}
