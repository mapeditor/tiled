import qbs 1.0

Module {
    Depends { name: "cpp" }
    cpp.includePaths: ["/home/kash/github/RadialGM/Submodules/enigma-dev/CommandLine/libEGM",
    "/home/kash/github/RadialGM/Submodules/enigma-dev/shared/protos/.eobjs",
    "/home/kash/github/RadialGM/Submodules/enigma-dev/shared"]
    Properties {
        cpp.staticLibraries: ["/home/kash/github/RadialGM/Submodules/enigma-dev/libEGM.so",
        "/home/kash/github/RadialGM/Submodules/enigma-dev/libENIGMAShared.so",
        "/home/kash/github/RadialGM/Submodules/enigma-dev/libProtocols.so",
        "/home/kash/github/RadialGM/Submodules/enigma-dev/libcompileEGMf.so"]
    }
}
