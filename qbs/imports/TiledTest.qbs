import qbs.FileInfo

CppApplication {
    type: base.concat("autotest")

    Depends { name: "libtiled" }
    Depends { name: "Qt.testlib" }
    Depends { name: "autotest" }

    cpp.cxxLanguageVersion: "c++17"
    cpp.cxxFlags: {
        var flags = base;
        if (qbs.toolchain.contains("msvc")) {
            if (Qt.core.versionMajor >= 6 && Qt.core.versionMinor >= 3)
                flags.push("/permissive-");
        }
        return flags;
    }
    cpp.rpaths: FileInfo.joinPaths(cpp.rpathOrigin, "../install-root/usr/local", project.libDir)
    autotest.workingDir: sourceDirectory
}
