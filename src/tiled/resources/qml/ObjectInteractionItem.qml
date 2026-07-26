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

        onTriggered: {
            borderDashOffset -= 0.5
        }
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
    }
}
