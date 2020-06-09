import qbs
import qbs.Environment
import qbs.File
import qbs.FileInfo
import qbs.Process
import qbs.Utilities

Probe {
    id: pythonDllProbe
    property string pythonDir: pythonInstallDir // Input
    property string buildVariant: qbs.buildVariant // Input
    property string fileNamePrefix // Output

    configure: {
        function printWarning(msg) {
            console.warn(msg + " The Python plugin will not be available.");
        }

        if (!pythonDir) {
            printWarning("PYTHONHOME not set.");
            return;
        }
        if (!File.exists(pythonDir)) {
            printWarning("The provided Python installation directory '" + pythonDir
                         + "' does not exist.");
            return;
        }
        var p = new Process();
        try {
            var pythonFilePath = FileInfo.joinPaths(pythonDir, "python.exe");
            p.exec(pythonFilePath, ["--version"], true);
            var output = p.readStdOut().trim();
            var magicPrefix = "Python ";
            if (!output.startsWith(magicPrefix)) {
                printWarning("Unexpected python output when checking for version: '"
                             + output + "'");
                return;
            }
            var versionNumberString = output.slice(magicPrefix.length);
            var versionNumbers = versionNumberString.split('.');
            if (versionNumbers.length < 2) {
                printWarning("Unexpected python output when checking for version: '"
                        + output + "'");
                return;
            }
            if (Utilities.versionCompare(versionNumberString, "3.5") < 0) {
                printWarning("The Python installation at '" + pythonDir
                             + "' has version " + versionNumberString + ", but 3.5 or higher "
                             + "is required.");
                return;
            }
            found = true;
            fileNamePrefix = "python" + versionNumbers[0] + versionNumbers[1];
            if (buildVariant === "debug")
                fileNamePrefix += "_d"
        } finally {
            p.close();
        }
    }
}
