import QtQuick
import QtQuick.Shapes
import QtQuick.Controls
import org.mapeditor.Tiled as Tiled

Tiled.RegionOverlay {
    id: regionOverlay

    property var scale
    property var regionPolygons

    onRegionChanged: {
        regionOverlay.regionPolygons = regionOverlay.polygons()
    }

    Shape {
        ShapePath {
            strokeColor: regionOverlay.strokeColor()
            strokeWidth: 1 / regionOverlay.scale
            fillColor: regionOverlay.fillColor()

            PathMultiline {
                paths: regionOverlay.regionPolygons
            }
        }
    }
}
