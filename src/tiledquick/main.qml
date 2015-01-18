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
        property alias windowX: window.x
        property alias windowY: window.y
        property alias windowWidth: window.width
        property alias windowHeight: window.height
        property alias windowVisibility: window.visibility
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
            containerAnimation.stop()
            containerAnimation.x += dx
            containerAnimation.y += dy
            containerAnimation.start()
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
