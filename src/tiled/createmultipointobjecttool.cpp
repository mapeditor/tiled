#include "createmultipointobjecttool.h"
#include "preferences.h"
#include "Utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "mapscene.h"
#include <QApplication>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

CreateMultipointObjectTool::CreateMultipointObjectTool(QObject* parent)
    : CreateObjectTool(CreateObjectTool::CreateGeometry, parent)
{
    mOverlayPolygonObject = new MapObject;

    mOverlayObjectGroup = new ObjectGroup;
    mOverlayObjectGroup->addObject(mOverlayPolygonObject);

    QColor highlight = QApplication::palette().highlight().color();
    mOverlayObjectGroup->setColor(highlight);
}

void CreateMultipointObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos,
                                                               Qt::KeyboardModifiers,
                                                               const bool snapToGrid, const bool snapToFineGrid)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    QPointF tileCoords = renderer->screenToTileCoords(pos);

    if (snapToFineGrid) {
        int gridFine = Preferences::instance()->gridFine();
        tileCoords = (tileCoords * gridFine).toPoint();
        tileCoords /= gridFine;
    } else if (snapToGrid)
        tileCoords = tileCoords.toPoint();

    QPointF pixelCoords = renderer->tileToPixelCoords(tileCoords);
    pixelCoords -= mNewMapObjectItem->mapObject()->position();

    QPolygonF polygon = mOverlayPolygonObject->polygon();
    polygon.last() = pixelCoords;
    mOverlayPolygonItem->setPolygon(polygon);
}

void CreateMultipointObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event,
                                                                 const bool, const bool)
{
    if (event->button() == Qt::RightButton) {
        finishNewMapObject();
    } else if (event->button() == Qt::LeftButton) {
        QPolygonF current = mNewMapObjectItem->mapObject()->polygon();
        QPolygonF next = mOverlayPolygonObject->polygon();

        // If the last position is still the same, ignore the click
        if (next.last() == current.last())
            return;

        // Assign current overlay polygon to the new object
        mNewMapObjectItem->setPolygon(next);

        // Add a new editable point to the overlay
        next.append(next.last());
        mOverlayPolygonItem->setPolygon(next);
    }
}

void CreateMultipointObjectTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    CreateObjectTool::startNewMapObject(pos, objectGroup);
    MapObject* newMapObject = mNewMapObjectItem->mapObject();
    QPolygonF polygon;
    polygon.append(QPointF());
    newMapObject->setPolygon(polygon);

    polygon.append(QPointF()); // The last point is connected to the mouse
    mOverlayPolygonObject->setPolygon(polygon);
    mOverlayPolygonObject->setShape(newMapObject->shape());
    mOverlayPolygonObject->setPosition(pos);

    mOverlayPolygonItem = new MapObjectItem(mOverlayPolygonObject,
                                            mapDocument());
    mapScene()->addItem(mOverlayPolygonItem);
}
