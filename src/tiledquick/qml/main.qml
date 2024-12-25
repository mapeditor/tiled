import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Window 2.11
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
            fitMapInView(false);
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

        // TODO: Eventually should be remembered for each map
        property alias mapScale: mapContainer.scale
        property alias mapX: mapContainer.x
        property alias mapY: mapContainer.y
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

    // File
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

    // View
    Action {
        id: fitMapInViewAction
        text: qsTr("Fit Map In View")
        shortcut: "Ctrl+/"
        onTriggered: fitMapInView();
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem { action: openAction }
            MenuSeparator {}
            MenuItem { action: exitAction }
        }
        Menu {
            title: qsTr("View")
            MenuItem { action: fitMapInViewAction }
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

        scale: 1 / Screen.devicePixelRatio
        width: parent.width * Screen.devicePixelRatio
        height: parent.height * Screen.devicePixelRatio
        transformOrigin: Item.TopLeft

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
            dx *= Screen.devicePixelRatio
            dy *= Screen.devicePixelRatio

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
            const scaleFactor = Math.pow(1.4, wheel.angleDelta.y / 120)

            let targetScale = containerAnimation.running ? containerAnimation.scale : mapContainer.scale
            targetScale = Math.min(8, Math.max(0.25, targetScale * scaleFactor))

            const anchor = mapToItem(mapContainer, wheel.x, wheel.y)
            const oldScale = mapContainer.scale
            const oldX = anchor.x * oldScale
            const oldY = anchor.y * oldScale
            const newX = anchor.x * targetScale
            const newY = anchor.y * targetScale

            containerAnimation.stop()
            containerAnimation.x = mapContainer.x - (newX - oldX)
            containerAnimation.y = mapContainer.y - (newY - oldY)
            containerAnimation.scale = targetScale
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

    function fitMapInView(animate = true) {
        // The amount that the map would need to be scaled by to fit within the view.
        let widthRatio = mapView.width / mapItem.width
        let heightRatio = mapView.height / mapItem.height
        // If we need to downscale the map to fit in the view, choose the lesser ratio
        // because that will result in the largest downscaling, which ensures the map
        // fits both vertically and horizontally.
        // If we need to upscale, we also want to choose the smaller ratio, as we want
        // both to fit.
        let scale = Math.min(widthRatio, heightRatio)
        containerAnimation.stop()
        if (animate) {
            containerAnimation.scale = scale
            containerAnimation.x = (mapView.width / 2) - ((mapItem.width * scale) / 2)
            containerAnimation.y = (mapView.height / 2) - ((mapItem.height * scale) / 2)
            containerAnimation.start()
        } else {
            mapContainer.scale = scale
            mapContainer.x = (mapView.width / 2) - ((mapItem.width * scale) / 2)
            mapContainer.y = (mapView.height / 2) - ((mapItem.height * scale) / 2)
        }
    }
}
