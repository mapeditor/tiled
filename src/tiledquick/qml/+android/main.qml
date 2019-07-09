import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import org.mapeditor.Tiled 1.0 as Tiled
import Qt.labs.settings 1.0

ApplicationWindow {
    id: window

    visible: true
    visibility: Window.FullScreen
    width: 640
    height: 480

    title: qsTr("Tiled Quick")

    FileDialog {
        id: fileDialog
        nameFilters: [ "TMX files (*.tmx)", "All files (*)" ]
        onAccepted: {
            mapLoader.source = fileDialog.fileUrl
            settings.mapsFolder = fileDialog.folder
        }
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


    Text {
        text: mapLoader.status === Tiled.MapLoader.Null ? qsTr("No map file loaded") : mapLoader.error
        anchors.centerIn: parent
    }

    Rectangle {
        color: Qt.rgba(255, 255, 255, 0.75)
        height: button.height + 10
        anchors.left: parent.left
        anchors.right: parent.right

        Row {
            spacing: 5
            Button {
                id: button
                x: 5; y: 5
                action: openAction
            }
        }
    }
}
