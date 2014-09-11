#include "createtileobjecttool.h"
#include "preferences.h"
#include "Utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "tile.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateTileObjectTool::CreateTileObjectTool(QObject* parent)
    : CreateObjectTool(CreateObjectTool::CreateTile, parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-image.png")));
    Utils::setThemeIcon(this, "insert-image");
    languageChanged();
}

void CreateTileObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers,
                                                         const bool snapToGrid, const bool snapToFineGrid)
{
    const MapRenderer *renderer = mapDocument()->renderer();

    const QSize imgSize = mNewMapObjectItem->mapObject()->cell().tile->size();
    const QPointF diff(-imgSize.width() / 2, imgSize.height() / 2);
    QPointF tileCoords = renderer->screenToTileCoords(pos + diff);

    if (snapToFineGrid) {
        int gridFine = Preferences::instance()->gridFine();
        tileCoords = (tileCoords * gridFine).toPoint();
        tileCoords /= gridFine;
    } else if (snapToGrid)
        tileCoords = tileCoords.toPoint();

    QPointF pixelCoords = renderer->tileToPixelCoords(tileCoords);

    mNewMapObjectItem->mapObject()->setPosition(pixelCoords);
    mNewMapObjectItem->syncWithMapObject();
    mNewMapObjectItem->setZValue(10000); // sync may change it
}

void CreateTileObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event,
                                                           const bool, const bool)
{
    if (event->button() == Qt::RightButton)
        cancelNewMapObject();
    CreateObjectTool::mousePressed(event);
}

void CreateTileObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event,
                                                            const bool, const bool)
{
    if (event->button() == Qt::LeftButton) {
        finishNewMapObject();
    }
}

void CreateTileObjectTool::languageChanged()
{
    setName(tr("Insert Tile"));
    setShortcut(QKeySequence(tr("T")));
}

MapObject* CreateTileObjectTool::createNewMapObject()
{
    if(!mTile)
        return 0;

    MapObject* newMapObject = new MapObject();
    newMapObject->setShape(MapObject::Rectangle);
    newMapObject->setCell(Cell(mTile));
    return newMapObject;
}
