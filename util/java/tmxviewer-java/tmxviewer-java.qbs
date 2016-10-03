import qbs

Product {
    Depends { name: "java"; required: false }
    Depends { name: "libtiled-java" }

    type: ["java.jar"]
    condition: java.present

    property string entryPoint: "TMXViewer"

    files: ["src/main/java/**/*.java"]
}
