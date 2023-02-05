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
    cpp.cxxFlags: {
        var flags = base;
        if (qbs.toolchain.contains("msvc")) {
            if (Qt.core.versionMajor >= 6 && Qt.core.versionMinor >= 3)
                flags.push("/permissive-");
        }
        return flags;
    }
    cpp.defines: [
        "QT_DISABLE_DEPRECATED_BEFORE=QT_VERSION_CHECK(5,15,0)",
        "QT_NO_DEPRECATED_WARNINGS",
        "QT_NO_CAST_FROM_ASCII",
        "QT_NO_CAST_TO_ASCII",
        "QT_NO_FOREACH",
        "QT_NO_URL_CAST_FROM_STRING",
    ]

    Group {
        qbs.install: true
        qbs.installDir: {
            if (project.windowsLayout) {
                return "";
            } else if (qbs.targetOS.contains("darwin")) {
                // Non-bundle applications are installed into the main Tiled.app bundle
                return bundle.isBundle ? "." : "Tiled.app/Contents/MacOS";
            } else {
                return "bin";
            }
        }
        qbs.installSourceBase: product.buildDirectory
        fileTagsFilter: {
            if (qbs.targetOS.contains("darwin") && bundle.isBundle)
                return ["bundle.content"];
            return product.type;
        }
    }
}
