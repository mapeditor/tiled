import qbs 1.0

Module {
    Depends { name: "cpp" }
    cpp.includePaths: ["/home/kash/github/RadialGM/Submodules/enigma-dev/CommandLine/libEGM"]
    Properties {
        cpp.staticLibraries: ["/home/kash/github/RadialGM/Submodules/enigma-dev/libEGM.so",
        "/home/kash/github/RadialGM/Submodules/enigma-dev/libENIGMAShared.so",
        "/home/kash/github/RadialGM/Submodules/enigma-dev/libProtocols.so",
        "/home/kash/github/RadialGM/Submodules/enigma-dev/libcompileEGMf.so"]
    }
}
