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

void CreateTileObjectTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers){
    CreateObjectTool::mouseMoved(pos, modifiers);
    if(!mNewMapObjectItem)
        return;

    const MapRenderer *renderer = mapDocument()->renderer();
    bool snapToGrid = Preferences::instance()->snapToGrid();
    bool snapToFineGrid = Preferences::instance()->snapToFineGrid();
    if (modifiers & Qt::ControlModifier) {
        snapToGrid = !snapToGrid;
        snapToFineGrid = false;
    }

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

void CreateTileObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        cancelNewMapObject();
    CreateObjectTool::mousePressed(event);
}

void CreateTileObjectTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    CreateObjectTool::mouseReleased(event);
    if (event->button() == Qt::LeftButton && mNewMapObjectItem) {
        finishNewMapObject();
    }
}

void CreateTileObjectTool::languageChanged()
{
    setName(tr("Insert Tile"));
    setShortcut(QKeySequence(tr("T")));
}

MapObject* CreateTileObjectTool::createNewMapObject(const QPointF &pos)
{
    if(!mTile)
        return 0;

    MapObject* newMapObject = new MapObject();
    newMapObject->setShape(MapObject::Rectangle);
    newMapObject->setCell(Cell(mTile));
    return newMapObject;
}
