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

        ShapePath {
            id: primaryPath

            fillColor: "transparent"
            strokeColor: "black"
            strokeWidth: 2 / mapContainer.scale

            strokeStyle: ShapePath.DashLine
            dashPattern: [1, 3]
            dashOffset: borderDashOffset

            PathMultiline {
                paths: objectInteractionItem.selectionOutlines
            }
        }

        ShapePath {
            fillColor: "transparent"
            strokeColor: "white"
            strokeWidth: primaryPath.strokeWidth

            strokeStyle: primaryPath.strokeStyle
            dashPattern: primaryPath.dashPattern
            dashOffset: primaryPath.dashOffset + primaryPath.dashPattern[1] - primaryPath.dashPattern[0]

            PathMultiline {
                paths: objectInteractionItem.selectionOutlines
            }
        }
    }
}
