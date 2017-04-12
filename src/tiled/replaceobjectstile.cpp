#include "replaceobjectstile.h"

#include "mapdocument.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

ReplaceObjectsTile::ReplaceObjectsTile(MapDocument *mapDocument,
                                       const QList<MapObject *> &mapObjects,
                                       Tile *tile)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Replace %n Object/s Tile",
                                               nullptr, mapObjects.size()))
    , mMapDocument(mapDocument)
    , mMapObjects(mapObjects)
    , mTile(tile)
{
    for (MapObject *object : mMapObjects)
        mOriginalTiles.append(object->cell().tile());
}

static void setObjectTile(MapObject *object, Tile *tile)
{
    Cell cell = object->cell();
    cell.setTile(tile);
    object->setCell(cell);
}

void ReplaceObjectsTile::replace()
{
    Tile * const tile = mTile;

    for (MapObject *object : mMapObjects)
        setObjectTile(object, tile);

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}

void ReplaceObjectsTile::restore()
{
    for (int i = 0; i < mMapObjects.size(); ++i)
        setObjectTile(mMapObjects[i], mOriginalTiles[i]);

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}

} // namespace Internal
} // namespace Tiled
