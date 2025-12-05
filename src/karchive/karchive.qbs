import qbs.FileInfo
import qbs.Utilities

StaticLibrary {
    builtByDefault: false   // only when needed by RpMap plugin
    condition: {
        if (qbs.toolchain.contains("msvc"))
            return false;

        if (Utilities.versionCompare(Qt.core.version, "6.8") < 0)
            return false;

        return true;
    }

    Depends { name: "cpp" }
    Depends { name: "Qt.core"; versionAtLeast: "6.8" }

    cpp.includePaths: [ "src" ]
    cpp.defines: [
        "KARCHIVE_STATIC_DEFINE",
        "QT_NO_CAST_FROM_ASCII",
    ]

    files : [
        "src/config-compression.h",
//        "src/k7zip.cpp",  // requires xz to build
        "src/k7zip.h",
        "src/kar.cpp",
        "src/kar.h",
        "src/karchivedirectory.h",
        "src/karchiveentry.h",
        "src/karchivefile.h",
        "src/karchive_export.h",
        "src/karchive_p.h",
        "src/karchive.cpp",
        "src/karchive.h",
        "src/kbzip2filter.cpp",
        "src/kbzip2filter.h",
        "src/kcompressiondevice.cpp",
        "src/kcompressiondevice.h",
        "src/kcompressiondevice_p.h",
        "src/kfilterbase.cpp",
        "src/kfilterbase.h",
        "src/kgzipfilter.cpp",
        "src/kgzipfilter.h",
        "src/klimitediodevice.cpp",
        "src/klimitediodevice_p.h",
        "src/klzfilter.cpp",
        "src/klzfilter.h",
        "src/knonefilter.cpp",
        "src/knonefilter.h",
        "src/krcc.cpp",
        "src/krcc.h",
        "src/ktar.cpp",
        "src/ktar.h",
//        "src/kxzfilter.cpp",
        "src/kxzfilter.h",
        "src/kzipfileentry.h",
        "src/kzstdfilter.cpp",
        "src/kzstdfilter.h",
        "src/kzip.cpp",
        "src/kzip.h",
        "src/loggingcategory.cpp",
        "src/loggingcategory.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: FileInfo.joinPaths(exportingProduct.sourceDirectory, "src")
        cpp.defines: [
            "KARCHIVE_NO_DEPRECATED",
            "KARCHIVE_STATIC_DEFINE",
        ]
    }
}
