import qbs
import qbs.FileInfo

CppApplication {
    type: base.concat("autotest")

    Depends { name: "libtiled" }
    Depends { name: "Qt.testlib" }
    Depends { name: "autotest" }

    cpp.cxxLanguageVersion: "c++17"
    cpp.rpaths: FileInfo.joinPaths(cpp.rpathOrigin, "../install-root/usr/local/lib/")
    autotest.workingDir: sourceDirectory
}
