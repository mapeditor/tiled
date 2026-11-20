InstallPackage {
    builtByDefault: project.snapshot || project.release
    condition: {
        return ((project.snapshot || project.release) &&
                qbs.targetOS.contains("windows"));
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

    property string bits: {
        if (qbs.architecture === "x86_64")
            return "64";
        if (qbs.architecture === "x86")
            return "32";
        if (qbs.architecture === "arm64")
            return "arm64";
        return "";
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
