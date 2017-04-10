import qbs

Product {
    Depends { name: "java"; required: false }
    condition: java.present && !(qbs.versionMinor == 6)

    type: ["java.jar"]
    files: ["src/main/java/**/*.java"]

    Export {
        Depends { name: "java"; required: false }
        java.manifestClassPath: [product.targetName + ".jar"]
    }
}
