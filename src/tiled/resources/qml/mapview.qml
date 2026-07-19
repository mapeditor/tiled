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

    property var mapEditor: mapEditorInstance

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

                map: mapItemMap
                scale: mapContainer.scale
                visibleArea: {
                    var scale = mapContainer.scale
                    Qt.rect(-mapContainer.x / scale,
                            -mapContainer.y / scale,
                            mapView.width / scale,
                            mapView.height / scale)
                }
            }

            RegionOverlay {
                id: selectedRegionOverlay
                anchors.fill: mapItem

                scale: mapContainer.scale
                region: mapEditor.selectedRegion
                tileSize: Qt.point(mapItem.map.tileWidth, mapItem.map.tileHeight)

                regionAlpha: 127
            }

            Tiled.MapBorderItem {
                id: mapBorderItem
                anchors.fill: mapItem

                color: "black"
            }

            Tiled.MapGridItem {
                id: mapGridItem
                anchors.fill: mapItem

                tileSize: Qt.point(mapItem.map.tileWidth, mapItem.map.tileHeight)
                scale: mapContainer.scale

                color: "black"
            }

            Tiled.MapItem { // Tool Brush
                id: toolBrush
                anchors.left: mapItem.left
                anchors.top: mapItem.top
                visible: singleFingerPanArea.containsMouse || singleFingerPanArea.pressed

                property var toolPreviewMap: mapEditor.tileEditPreview
                map: toolPreviewMap

                visibleArea: {
                    // TODO: Adjust to only show needed visible area
                    if (this.map)
                        Qt.rect(0,
                                0,
                                this.width,
                                this.height)
                    else
                        Qt.rect(0, 0, 0, 0)
                }
            }

            RegionOverlay {
                id: brushRegionOverlay
                anchors.fill: mapItem
                visible: toolBrush.visible

                scale: mapContainer.scale
                region: mapEditor.tileEditRegion
                mapRect: Qt.rect(0, 0, mapItem.map.width, mapItem.map.height)
                tileSize: Qt.point(mapItem.map.tileWidth, mapItem.map.tileHeight)
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

        onPressed: (event) => mapEditor.quickMousePressed(
            event.button,
            event.buttons,
            event.modifiers,
            singleFingerPanArea.mapToItem(mapItem, event.x, event.y),
            singleFingerPanArea.mapToItem(null, event.x, event.y),
            singleFingerPanArea.mapToGlobal(event.x, event.y)
        )

        onReleased: (event) => mapEditor.quickMouseReleased(
            event.button,
            event.buttons,
            event.modifiers,
            singleFingerPanArea.mapToItem(mapItem, event.x, event.y),
            singleFingerPanArea.mapToItem(null, event.x, event.y),
            singleFingerPanArea.mapToGlobal(event.x, event.y)
        )

        onPositionChanged: (event) => mapEditor.quickMouseMoved(
            singleFingerPanArea.mapToItem(mapItem, event.x, event.y),
            event.modifiers
        )

        onContainsMouseChanged: mapEditor.quickContainsMouseChanged(singleFingerPanArea.containsMouse)
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
