import qbs

InstallPackage {
    builtByDefault: false
    condition: {
        return ((project.snapshot || project.release) &&
                (qbs.toolchain.contains("mingw") || qbs.toolchain.contains("msvc")) ||
                (project.linuxArchive && qbs.targetOS.contains("linux")));
    }

    archiver.type: {
        if (qbs.targetOS.contains("windows"))
            return "7zip"
        else
            return "tar"
    }

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

    targetName: {
        var baseName = "Tiled-" + project.version;
        if (qbs.targetOS.contains("windows"))
            baseName += "-win";
        else if (qbs.targetOS.contains("linux"))
            baseName += "-linux";
        return baseName + bits;
    }
}
