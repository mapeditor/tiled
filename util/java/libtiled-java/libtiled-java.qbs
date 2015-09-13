import qbs

JavaJarFile {

    files: ["src/**/*.java"]

    Export {
        Depends { name: "java" }
        java.manifestClassPath: [product.targetName + ".jar"]
    }
}
