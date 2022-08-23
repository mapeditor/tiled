import qbs 1.0

Module {
    Depends { name: "cpp" }

    cpp.includePaths: ["/home/kash/github/RadialGM/Submodules/enigma-dev/CommandLine/libEGM",
    "/home/kash/github/RadialGM/Submodules/enigma-dev/shared/protos/.eobjs",
    "/home/kash/github/RadialGM/Submodules/enigma-dev/shared",
    "/home/kash/github/RadialGM/Submodules/enigma-dev/shared/event_reader"]

    /*cpp.dynamicLibraries: ["/home/kash/github/RadialGM/Submodules/enigma-dev/libEGM.so",
    "/home/kash/github/RadialGM/Submodules/enigma-dev/libENIGMAShared.so",
    "/home/kash/github/RadialGM/Submodules/enigma-dev/libProtocols.so",
    "/home/kash/github/RadialGM/Submodules/enigma-dev/libcompileEGMf.so"]*/

    cpp.libraryPaths: ["/home/kash/github/RadialGM/Submodules/enigma-dev"]

    cpp.rpaths: ["/home/kash/github/RadialGM/Submodules/enigma-dev"]

    cpp.dynamicLibraries: ["EGM", "ENIGMAShared", "Protocols", "compileEGMf"]

    /*Properties {
        cpp.libraryPaths: ["/home/kash/github/RadialGM/Submodules/enigma-dev"]

        cpp.dynamicLibraries: ["EGM", "ENIGMAShared", "Protocols", "compileEGMf"]
    }*/
}
