/*
  Determines which files of the QML modules shipped with Tiled and which of
  the Qt Quick libraries they need are available in the Qt installation.

  Shared between the "distribute" product, which puts these files in the
  archive, and the Windows installer, which ships them in the MSI. Both are
  based on the qmlImportDirs and qtQuickLibraries properties in tiled.qbs.
*/

var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");

/**
 * Returns an entry for each of the given import directories that exists below
 * qmlDir, providing the directory (both relative and absolute) along with the
 * names of the files to ship from it.
 */
function moduleDirs(qmlDir, importDirs)
{
    var result = [];

    for (var i = 0; i < importDirs.length; ++i) {
        var dir = importDirs[i];
        var absDir = FileInfo.joinPaths(qmlDir, dir);
        if (!File.exists(absDir))
            continue;

        var files = [];
        var entries = File.directoryEntries(absDir, File.Files);
        for (var j = 0; j < entries.length; ++j) {
            var entry = entries[j];
            if (entry.endsWith(".qmltypes") || entry.endsWith(".pdb"))
                continue;

            // Skip debug variants of the QML plugins
            if (entry.endsWith("d.dll") && File.exists(FileInfo.joinPaths(absDir, entry.slice(0, -5) + ".dll")))
                continue;

            files.push(entry);
        }

        result.push({ dir: dir, absDir: absDir, files: files });
    }

    return result;
}

/**
 * Returns the files to ship for the given import directories, as paths
 * relative to qmlDir.
 */
function moduleFiles(qmlDir, importDirs)
{
    var result = [];
    var dirs = moduleDirs(qmlDir, importDirs);

    for (var i = 0; i < dirs.length; ++i) {
        var files = dirs[i].files;
        for (var j = 0; j < files.length; ++j)
            result.push(dirs[i].dir + "/" + files[j]);
    }

    return result;
}

/**
 * Returns the file names of the given Qt Quick libraries which exist in the
 * directory referred to by prefix (which is expected to include the trailing
 * separator). Their existence depends on the Qt version.
 */
function existingLibraries(prefix, versionMajor, libraries, suffix)
{
    var result = [];

    for (var i = 0; i < libraries.length; ++i) {
        var lib = "Qt" + versionMajor + libraries[i] + suffix;
        if (File.exists(prefix + lib))
            result.push(lib);
    }

    return result;
}
