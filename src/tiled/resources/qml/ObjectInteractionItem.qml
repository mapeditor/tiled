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

    SystemPalette {
        id: systemPalette
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

        ShapePath {
            id: objectHandleArrows

            fillColor: "black"
            strokeColor: "white"
            strokeWidth: 1 / mapContainer.scale

            PathMultiline {
                paths: objectHandlePolygons
            }
        }

        ShapePath {
            id: hoveredHandle

            fillColor: "white"
            strokeColor: "black"
            strokeWidth: 1 / mapContainer.scale

            PathPolyline {
                path: hoveredHandlePolygon
            }
        }
    }

    // EditPolygonTool
    property var highlightedSelectedPolygonEditPoints: innerJoin(objectInteractionItem.selectedPolygonEditPoints ?? [], objectInteractionItem.highlightedPolygonEditPoints ?? [])
    property real editPointSize: 8 / mapContainer.scale
    Repeater {
        id: defaultEditPoints
        model: objectInteractionItem.polygonEditPoints

        delegate: Rectangle {
            x: modelData.x - editPointSize/2
            y: modelData.y - editPointSize/2

            width: editPointSize
            height: editPointSize
            radius: editPointSize/2

            color: "lightgray"

            border.pixelAligned: false
            border.color: "black"
            border.width: width/8
        }
    }
    Repeater {
        id: highlightedEditPoints
        model: objectInteractionItem.highlightedPolygonEditPoints
        delegate: Rectangle {
            x: modelData.x - editPointSize/2
            y: modelData.y - editPointSize/2

            width: editPointSize
            height: editPointSize
            radius: editPointSize/2

            color: Qt.lighter("lightgray")

            border.pixelAligned: false
            border.color: "black"
            border.width: width/8
        }
    }
    Repeater {
        id: selectedEditPoints
        model: objectInteractionItem.selectedPolygonEditPoints

        delegate: Rectangle {
            x: modelData.x - 1.5 * editPointSize/2
            y: modelData.y - 1.5 * editPointSize/2

            width: 1.5 * editPointSize
            height: 1.5 * editPointSize
            radius: 1.5 * editPointSize/2

            color: systemPalette.highlight

            border.pixelAligned: false
            border.color: "black"
            border.width: width/8
        }
    }
    Repeater {
        id: highlightedSelectedEditPoints
        model: highlightedSelectedPolygonEditPoints

        delegate: Rectangle {
            x: modelData.x - 1.5 * editPointSize/2
            y: modelData.y - 1.5 * editPointSize/2

            width: 1.5 * editPointSize
            height: 1.5 * editPointSize
            radius: 1.5 * editPointSize/2

            color: Qt.lighter(systemPalette.highlight)

            border.pixelAligned: false
            border.color: "black"
            border.width: width/8
        }
    }

    function innerJoin(listA, listB) {
        let mapA = {}
        let output = []

        for (let a = 0; a < listA.length; a++)
            mapA[listA[a]] = true

        for (let b = 0; b < listB.length; b++)
            if (mapA[listB[b]])
                output.push(listB[b])

        return output
    }

    // function leftAntiJoin(listA, listB) {
    //     let mapB = {}
    //     let output = []

    //     for (let b = 0; b < listB.length; b++)
    //         mapB[listB[b]] = true

    //     for (let a = 0; a < listA.length; a++) {
    //         if (!mapB[listA[a]]) {
    //             output.push(listA[a])
    //         }
    //     }

    //     return output
    // }
}
