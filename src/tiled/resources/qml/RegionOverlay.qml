import QtQuick
import QtQuick.Shapes
import org.mapeditor.Tiled as Tiled

Tiled.RegionOverlay {
    id: regionOverlay

    property var scale
    // TODO: Draw only visible area

    Shape {
        ShapePath {
            strokeColor: regionOverlay.validStrokeColor()
            strokeWidth: 1 / regionOverlay.scale
            fillColor: regionOverlay.validFillColor()

            PathMultiline {
                paths: regionOverlay.validPolygons
            }
        }

        ShapePath {
            strokeColor: regionOverlay.invalidStrokeColor()
            strokeWidth: 1 / regionOverlay.scale
            fillColor: regionOverlay.invalidFillColor()

            PathMultiline {
                paths: regionOverlay.invalidPolygons
            }
        }
    }
}
