import QtQuick
import QtQuick.Shapes
import org.mapeditor.Tiled as Tiled

Tiled.ObjectInteractionItem {
    id: objectInteractionItem

    property real borderDashOffset: 0
    Timer {
        id: timer
        interval: 50
        running: true
        repeat: true

        onTriggered: borderDashOffset -= 0.5
    }

    Shape {
        anchors.fill: parent

        property var selectionOutlines: objectInteractionItem.selectionOutlines
        ShapePath {
            id: primarySelectionOutline

            fillColor: "transparent"
            strokeColor: "black"
            strokeWidth: 2 / mapContainer.scale

            strokeStyle: ShapePath.DashLine
            dashOffset: borderDashOffset
            dashPattern: [2, 4]

            PathMultiline {
                paths: selectionOutlines
            }
        }
        ShapePath {
            id: secondarySelectionOutline

            fillColor: "transparent"
            strokeColor: "white"
            strokeWidth: primarySelectionOutline.strokeWidth

            strokeStyle: primarySelectionOutline.strokeStyle
            dashOffset: primarySelectionOutline.dashOffset + 3
            dashPattern: primarySelectionOutline.dashPattern

            PathMultiline {
                paths: selectionOutlines
            }
        }

        property var hoverOutlines: objectInteractionItem.hoverOutlines
        ShapePath {
            id: primaryHoverOutline

            fillColor: "transparent"
            strokeColor: Qt.rgba(0, 0, 0, 0.5)
            strokeWidth: 2 / mapContainer.scale

            strokeStyle: ShapePath.DashLine
            dashOffset: 0
            dashPattern: [2, 4]

            PathMultiline {
                paths: hoverOutlines
            }
        }
        ShapePath {
            id: secondaryHoverOutline

            fillColor: "transparent"
            strokeColor: Qt.rgba(255, 255, 255, 0.5)
            strokeWidth: primaryHoverOutline.strokeWidth

            strokeStyle: primaryHoverOutline.strokeStyle
            dashOffset: 3
            dashPattern: primaryHoverOutline.dashPattern

            PathMultiline {
                paths: hoverOutlines
            }
        }

        ShapePath {
            id: selectionToolRectShadow

            fillColor: "transparent"
            strokeColor: Qt.rgba(0, 0, 0, 0.5)
            strokeWidth: 2 / mapContainer.scale

            strokeStyle: ShapePath.DashLine
            dashPattern: [1.5, 3]

            PathRectangle {
                x: objectInteractionItem.selectionRect.x + 1 / mapContainer.scale
                y: objectInteractionItem.selectionRect.y + 1 / mapContainer.scale
                width: objectInteractionItem.selectionRect.width
                height: objectInteractionItem.selectionRect.height
            }
        }
        ShapePath {
            id: selectionToolRect

            fillColor: objectInteractionItem.selectionRectFillColor()
            strokeColor: objectInteractionItem.selectionRectBorderColor()
            strokeWidth: 2 / mapContainer.scale

            strokeStyle: ShapePath.DashLine
            dashPattern: [1.5, 3]

            PathRectangle {
                x: objectInteractionItem.selectionRect.x
                y: objectInteractionItem.selectionRect.y
                width: objectInteractionItem.selectionRect.width
                height: objectInteractionItem.selectionRect.height
            }
        }
    }
}
