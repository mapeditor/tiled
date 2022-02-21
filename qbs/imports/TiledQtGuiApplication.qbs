import qbs

QtGuiApplication {
    cpp.useRPaths: project.useRPaths
    cpp.rpaths: {
        if (qbs.targetOS.contains("darwin"))
            return ["@loader_path/../Frameworks"];
        else
            return ["$ORIGIN/../lib"];
    }
    cpp.cxxLanguageVersion: "c++17"
    cpp.defines: [
        "QT_DISABLE_DEPRECATED_BEFORE=QT_VERSION_CHECK(5,15,0)",
        "QT_NO_DEPRECATED_WARNINGS",
        "QT_NO_FOREACH"
    ]

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
            if (qbs.targetOS.contains("windows"))
                return "";
            else if (qbs.targetOS.contains("darwin"))
                return "Tiled.app/Contents/MacOS";
            else
                return "bin";
        }

        fileTagsFilter: product.type
    }
}
