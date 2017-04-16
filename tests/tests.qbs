import qbs 1.0
import qbs.Environment

Project {
    references: [
        "quicktiledplugin/quicktiledplugin.qbs"
    ]

    AutotestRunner {
//        environment: {
//            base.concat([ "QML2_IMPORT_PATH=../../qml" ])
//            console.info("test---" + base.concat([ "QML2_IMPORT_PATH=../../qml" ]))c
//            console.info(JSON.stringify(Environment.currentEnv()))
//        }
    }
}
