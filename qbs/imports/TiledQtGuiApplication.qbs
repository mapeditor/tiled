import qbs

QtGuiApplication {
    cpp.useRPaths: project.useRPaths
    cpp.rpaths: {
        if (qbs.targetOS.contains("darwin"))
            return ["@loader_path/../Frameworks"];
        else if (project.linuxArchive)
            return ["$ORIGIN/lib"]
        else
            return ["$ORIGIN/../lib"];
    }
    cpp.cxxLanguageVersion: "c++11"

    Properties {
        condition: qbs.targetOS.contains("macos")
        cpp.cxxFlags: ["-Wno-unknown-pragmas"]
    }

    Group {
        condition: qbs.targetOS.contains("darwin") && bundle.isBundle
        qbs.install: true
        qbs.installSourceBase: product.buildDirectory
        fileTagsFilter: ["bundle.content"]
    }

    Group {
        condition: !qbs.targetOS.contains("darwin") || !bundle.isBundle
        qbs.install: true
        qbs.installDir: {
            if (qbs.targetOS.contains("windows") || project.linuxArchive)
                return "";
            else if (qbs.targetOS.contains("darwin"))
                return "Tiled.app/Contents/MacOS";
            else
                return "bin";
        }

        fileTagsFilter: product.type
    }
}
