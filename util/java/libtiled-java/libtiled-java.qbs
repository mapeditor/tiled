import qbs

Product {
    Depends { name: "java"; required: false }
    condition: java.present

    type: ["java.jar"]
    files: ["src/**/*.java"]

    Export {
        Depends { name: "java"; required: false }
        java.manifestClassPath: [product.targetName + ".jar"]
    }
}
