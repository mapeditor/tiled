import QtQuick 2.6
import QtTest 1.0

import org.mapeditor.Tiled 1.0 as Tiled

TestCase {
    name: "tst_quicktiledplugin"

    Tiled.MapLoader {
        id: mapLoader
        source: "doesnotexist.tmx"
    }

    SignalSpy {
        id: errorSpy
        target: mapLoader
        signalName: "errorChanged"
    }

    SignalSpy {
        id: statusSpy
        target: mapLoader
        signalName: "statusChanged"
    }

    function test_mapLoaderSignals() {
        // TODO: fix these failures
//        compare(errorSpy.count, 1);
//        compare(statusSpy.count, 1);
    }
}
