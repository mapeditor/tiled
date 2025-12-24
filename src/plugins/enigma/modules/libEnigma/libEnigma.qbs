import qbs
import qbs.Environment

Module {
    Depends { name: "cpp" }

    property string enigmaPath: Environment.getEnv("ENIGMA_PATH")

    cpp.includePaths: [enigmaPath + "/CommandLine/libEGM",
    enigmaPath + "/shared/protos/.eobjs",
    enigmaPath + "/shared",
    enigmaPath + "/shared/event_reader"]

    cpp.libraryPaths: [enigmaPath]

    cpp.rpaths: [enigmaPath]

    cpp.dynamicLibraries: ["EGM", "ENIGMAShared", "Protocols", "compileEGMf"]
}
