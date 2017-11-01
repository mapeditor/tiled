import qbs

Project {
    name: "tests"

    AutotestRunner {
        wrapper: ["xvfb-run", "-a"]
    }

    references: [
        "mapreader",
        "staggeredrenderer",
    ]
}

