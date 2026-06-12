import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs
import org.mapeditor.Tiled as Tiled
import QtCore

Rectangle {
    id: widget
    anchors.fill: parent
    visible: true

    Settings {
        id: settings
        property string mapsFolder

        // TODO: Eventually should be remembered for each map
        property alias mapScale: mapContainer.scale
        property alias mapX: mapContainer.x
        property alias mapY: mapContainer.y
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
                map: mapItemMap;

                visibleArea: {
                    var scale = mapContainer.scale
                    Qt.rect(-mapContainer.x / scale,
                            -mapContainer.y / scale,
                            mapView.width / scale,
                            mapView.height / scale);
                }

                // TODO: Remove if not needed later.
                // Component.onCompleted: {
                //     mapItem.map = mapItemMap
                // }
            }

            Tiled.MapBorderItem {
                id: mapBorderItem
                anchors.fill: mapItem

                color: "black"
            }

            Tiled.MapGridItem {
                id: mapGriditem
                anchors.fill: mapItem

                tileSize: Qt.point(mapItem.map.tileWidth, mapItem.map.tileHeight);
                scale: mapContainer.scale;

                color: "black"
            }
        }
    }

    DragArea {
        id: singleFingerPanArea
        anchors.fill: parent

        onDragged: function(dx, dy) {
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

        onWheel: function(wheel) {
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

    RowLayout {
        anchors.bottom: parent.bottom;

        Label {
            text: {
                var mapRelativeCoords = singleFingerPanArea.mapToItem(mapItem, singleFingerPanArea.mouseX, singleFingerPanArea.mouseY)
                var tileCoords = mapItem.screenToTileCoords(mapRelativeCoords.x, mapRelativeCoords.y)
                Math.floor(tileCoords.x) + ", " + Math.floor(tileCoords.y)
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
