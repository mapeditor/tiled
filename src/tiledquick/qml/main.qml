import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import org.mapeditor.Tiled 1.0 as Tiled
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0 as Platform

// For access to FontAwesome Singleton
import "."

ApplicationWindow {
    id: window
    width: 1024
    height: 720
    minimumWidth: 480
    minimumHeight: 320
    title: qsTr("Tiled Quick")
    visible: true

    FontLoader {
        id: fontAwesomeLoader
        source: "fonts/fontawesome.ttf"
    }

    Platform.FileDialog {
        id: fileDialog
        nameFilters: [ "TMX files (*.tmx)", "All files (*)" ]
        onAccepted: {
            mapLoader.source = fileDialog.file
            settings.mapsFolder = fileDialog.folder
        }
    }

    Platform.MessageDialog {
        id: aboutBox
        title: "About Tiled Quick"
        text: "This is an experimental Qt Quick version of Tiled,\na generic 2D map editor"
    }

    Settings {
        id: settings
        property string mapsFolder
        property alias mapSource: mapLoader.source
        property alias windowX: window.x
        property alias windowY: window.y
        property alias windowWidth: window.width
        property alias windowHeight: window.height
        property alias windowVisibility: window.visibility
    }

    Shortcut {
        sequence: "F11"
        onActivated: {
            if (window.visibility == ApplicationWindow.FullScreen)
                window.visibility = ApplicationWindow.Windowed
            else
                window.visibility = ApplicationWindow.FullScreen
        }
    }

    Action {
        id: openAction
        text: qsTr("Open...")
        shortcut: StandardKey.Open
        onTriggered: {
            fileDialog.folder = settings.mapsFolder
            fileDialog.open()
        }
    }

    Action {
        id: exitAction
        text: qsTr("Exit")
        shortcut: StandardKey.Quit
        onTriggered: Qt.quit()
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                action: openAction
                text: qsTr("Open...")
                onTriggered: {
                    fileDialog.open()
                }
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr("Exit")
                action: exitAction
                onTriggered: Qt.quit()
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

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                action: openAction
                font.family: fontAwesomeLoader.name
                text: FontAwesome.open
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

            ParallelAnimation {
                id: containerAnimation

                property alias scale: scaleAnimation.to
                property alias x: xAnimation.to
                property alias y: yAnimation.to

                NumberAnimation { id: scaleAnimation; target: mapContainer; property: "scale"; easing.type: Easing.OutCubic; duration: 100 }
                NumberAnimation { id: xAnimation; target: mapContainer; property: "x"; easing.type: Easing.OutCubic; duration: 100 }
                NumberAnimation { id: yAnimation; target: mapContainer; property: "y"; easing.type: Easing.OutCubic; duration: 100 }
            }

            Component.onCompleted: {
                containerAnimation.scale = mapContainer.scale
                containerAnimation.x = mapContainer.x
                containerAnimation.y = mapContainer.y
            }

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

    DragArea {
        id: singleFingerPanArea
        anchors.fill: parent

        onDragged: {
            if (containerAnimation.running) {
                containerAnimation.stop()
                containerAnimation.x += dx
                containerAnimation.y += dy
                mapContainer.x += dx
                mapContainer.y += dy
                containerAnimation.start()
            } else {
                mapContainer.x += dx
                mapContainer.y += dy
            }
        }

        onWheel: {
            var scaleFactor = Math.pow(1.4, wheel.angleDelta.y / 120)
            var scale = Math.min(8, Math.max(0.25, containerAnimation.scale * scaleFactor))
            var anchor = mapToItem(mapContainer, wheel.x, wheel.y)
            var oldScale = mapContainer.scale
            var oldX = anchor.x * oldScale
            var oldY = anchor.y * oldScale
            var newX = anchor.x * scale
            var newY = anchor.y * scale

            containerAnimation.stop()
            containerAnimation.x = mapContainer.x - (newX - oldX)
            containerAnimation.y = mapContainer.y - (newY - oldY)
            containerAnimation.scale = scale
            containerAnimation.start()
        }
    }

    footer: Pane {
        RowLayout {
            Label {
                text: {
                    if (mapLoader.status === Tiled.MapLoader.Null) {
                        qsTr("No map file loaded")
                    } else if (mapLoader.status === Tiled.MapLoader.Error) {
                        mapLoader.error
                    } else {
                        var mapRelativeCoords = singleFingerPanArea.mapToItem(mapItem, singleFingerPanArea.mouseX, singleFingerPanArea.mouseY)
                        var tileCoords = mapItem.screenToTileCoords(mapRelativeCoords.x, mapRelativeCoords.y)
                        Math.floor(tileCoords.x) + ", " + Math.floor(tileCoords.y)
                    }
                }
            }
        }
    }
}
