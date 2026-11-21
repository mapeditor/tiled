import qbs.File

// Enumerates the redistributable directory supplied by the build environment.
Probe {
    property path crtDir
    property stringList files

    configure: {
        if (!crtDir || !File.exists(crtDir))
            throw "Set TILED_MSVC_REDIST_DIR to the target architecture's CRT directory.";

        files = File.directoryEntries(crtDir, File.Files).filter(function (name) {
            return /^(msvcp|vcruntime|concrt|vccorlib|vcomp|vcamp)140.*\.dll$/i.test(name);
        });
        if (files.length === 0)
            throw "No MSVC runtime DLLs found in '" + crtDir + "'.";
        found = true;
    }
}
