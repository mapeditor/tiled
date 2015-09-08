import qbs

JavaJarFile {
    files: ["src/**/*.java"]

    Group {
        fileTagsFilter: ["java.jar"]
        qbs.install: true
        qbs.installDir: "jar"
    }

    Export {
        Depends { name: "java" }
        java.manifestClassPath: [product.targetName + ".jar"]
    }
}
