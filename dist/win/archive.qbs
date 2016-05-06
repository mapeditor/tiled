import qbs

InstallPackage {
    builtByDefault: false
    condition: {
        return (project.snapshot || project.release) &&
               (qbs.toolchain.contains("mingw") || qbs.toolchain.contains("msvc"));
    }

    archiver.type: "7zip"

    Depends {
        productTypes: [
            "application",
            "dynamiclibrary",
            "qm",
            "installable",
        ]
    }

    property int bits: {
        if (qbs.architecture === "x86_64")
            return 64;
        if (qbs.architecture === "x86")
            return 32;
    }

    targetName: "tiled-" + project.version + "-win" + bits
}
