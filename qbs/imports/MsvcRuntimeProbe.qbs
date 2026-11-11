import qbs.File
import qbs.FileInfo

// Locates the Visual Studio runtime DLLs matching the target architecture,
// so they can be shipped alongside Tiled.
Probe {
    id: msvcRuntimeProbe

    // Inputs
    property string toolchainInstallPath // like <VC>/Tools/MSVC/<ver>/bin/<host>/<arch>
    property string architecture

    // Outputs
    property path crtDir
    property stringList files

    configure: {
        function printWarning(msg) {
            console.warn(msg + " The MSVC runtime DLLs will not be packaged.");
        }

        var archDirName = {
            "x86": "x86",
            "x86_64": "x64",
            "arm64": "arm64"
        }[architecture] || architecture;

        var runtimeName = /^(msvcp|vcruntime|concrt|vccorlib|vcomp|vcamp)140.*\.dll$/i;

        // Candidate directories containing runtime DLLs
        var dirs = [];

        // Preferred: <VC>/Redist/MSVC/<ver>/<arch>/Microsoft.VC<ver>.CRT
        var msvcDir = FileInfo.joinPaths(toolchainInstallPath, "..", "..", "..");
        var redistBase = FileInfo.joinPaths(msvcDir, "..", "..", "Redist", "MSVC");
        if (File.exists(redistBase)) {
            var versions = File.directoryEntries(redistBase, File.Dirs);
            versions.sort().reverse();
            for (var i = 0; i < versions.length; ++i) {
                var archDir = FileInfo.joinPaths(redistBase, versions[i], archDirName);
                var crtDirs = File.directoryEntries(archDir, File.Dirs)
                        .filter(function (name) {
                            return name.match(/^Microsoft\.VC[0-9]+\.CRT$/);
                        });
                for (var j = 0; j < crtDirs.length; ++j)
                    dirs.push(FileInfo.joinPaths(archDir, crtDirs[j]));
            }
        }

        // Fallback: the compiler directory also contains the runtime DLLs
        dirs.push(toolchainInstallPath);

        for (var i = 0; i < dirs.length; ++i) {
            var dlls = File.directoryEntries(dirs[i], File.Files)
                    .filter(function (name) {
                        return name.match(runtimeName);
                    });
            if (dlls.length > 0) {
                crtDir = dirs[i];
                files = dlls;
                found = true;
                return;
            }
        }

        printWarning("Could not find the MSVC runtime DLLs under '"
                     + msvcDir + "'.");
    }
}
