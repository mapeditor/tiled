import qbs

JavaJarFile {
    Depends { name: "libtiled-java" }

    entryPoint: "TMXViewer"

    files: ["src/**/*.java"]

    Group {
        fileTagsFilter: ["java.jar"]
        qbs.install: true
        qbs.installDir: "jar"
    }
}
