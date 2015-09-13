import qbs

JavaJarFile {
    Depends { name: "libtiled-java" }

    entryPoint: "TMXViewer"

    files: ["src/**/*.java"]
}
