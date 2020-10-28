Project {
    name: "plugins"

    SubProject {
        filePath: "rpmap/rpmap.qbs"
        Properties {
            condition: parent.enableKArchive
        }
    }

    references: [
        "csv",
        "defold",
        "defoldcollection",
        "droidcraft",
        "flare",
        "gmx",
        "json",
        "json1",
        "lua",
        "python",
        "replicaisland",
        "tbin",
        "tengine"
    ]
}
