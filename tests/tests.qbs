import qbs.ModUtils

Project {
    name: "tests"

    references: [
        "automapping",
        "mapreader",
        "properties",
        "scriptmodule",
        "staggeredrenderer",
    ]

    AutotestRunner {
        environment: ModUtils.flattenDictionary(qbs.commonRunEnvironment)
                     .concat(["QT_QPA_PLATFORM=offscreen"])
    }
}
