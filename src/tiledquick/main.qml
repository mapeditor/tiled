import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import org.mapeditor.Tiled 1.0 as Tiled
import Qt.labs.settings 1.0

ApplicationWindow {
    id: window

    visible: true

    width: 1024
    height: 720

    minimumWidth: 480
    minimumHeight: 320

    title: qsTr("Tiled Quick")

    FileDialog {
        id: fileDialog
        nameFilters: [ "TMX files (*.tmx)", "All files (*)" ]
        onAccepted: {
            mapLoader.source = fileDialog.fileUrl
            settings.mapsFolder = fileDialog.folder
        }
    }

    MessageDialog {
        id: aboutBox
        title: "About Tiled Quick"
        text: "This is an experimental Qt Quick version of Tiled,\na generic 2D map editor"
        icon: StandardIcon.Information
    }

    Settings {
        id: settings
        property string mapsFolder
        property alias mapSource: mapLoader.source
    }

    Action {
        id: openAction
        text: qsTr("Open...")
        shortcut: StandardKey.Open
        iconName: "document-open"
        onTriggered: {
            fileDialog.folder = settings.mapsFolder
            fileDialog.open()
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                action: openAction
                text: qsTr("Open...")
                shortcut: StandardKey.Open
                onTriggered: {
                    fileDialog.open()
                }
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Exit")
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit();
            }
        }
        Menu {
            title: qsTr("Help")
            MenuItem {
                text: qsTr("About Tiled Quick")
                onTriggered: aboutBox.open()
            }
        }
    }

    toolBar: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                action: openAction
            }
        }
    }

    Tiled.MapLoader {
        id: mapLoader
    }

    Item {
        id: mapView
        anchors.fill: parent

        Item {
            id: mapContainer

            Tiled.MapItem {
                id: mapItem
                map: mapLoader.map
                visibleArea: {
                    var scale = mapContainer.scale
                    Qt.rect(-mapContainer.x / scale,
                            -mapContainer.y / scale,
                            mapView.width / scale,
                            mapView.height / scale);
                }
            }
        }
    }

    PinchArea {
        id: twoFingerPanAndZoomArea
        anchors.fill: parent

        property real startScale

        onPinchStarted: {
            startScale = mapContainer.scale
        }
        onPinchUpdated: {
            var mapCenter = mapToItem(mapContainer, pinch.center.x, pinch.center.y)
            var oldScale = mapContainer.scale
            var newScale = Math.min(8, Math.max(0.25, startScale * pinch.scale))
            var oldX = mapCenter.x * oldScale
            var oldY = mapCenter.y * oldScale
            var newX = mapCenter.x * newScale
            var newY = mapCenter.y * newScale
            mapContainer.x += (pinch.center.x - pinch.previousCenter.x) - (newX - oldX)
            mapContainer.y += (pinch.center.y - pinch.previousCenter.y) - (newY - oldY)
            mapContainer.scale = newScale
        }

        MouseArea {
            id: singleFingerPanArea
            anchors.fill: parent
            drag.target: mapContainer
        }
    }

    statusBar: StatusBar {
        RowLayout {
            Label {
                text: {
                    if (mapLoader.status === Tiled.MapLoader.Null)
                        qsTr("No map file loaded")
                    else if (mapLoader.status === Tiled.MapLoader.Error)
                        mapLoader.error
                    else
                        mapLoader.source
                }
            }
        }
    }
}
