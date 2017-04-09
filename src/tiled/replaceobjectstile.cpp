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
{}

void ReplaceObjectsTile::replace()
{
    Tile * const tile = mTile;

    mOldTiles.clear();

    for (MapObject *object : mMapObjects) {
        mOldTiles.append(object->cell().tile());
        swapObjectTile(object, tile);
    }

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}

void ReplaceObjectsTile::restore()
{
    for(int i=0; i<mMapObjects.size(); i++)
        swapObjectTile(mMapObjects[i], mOldTiles[i]);

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}

void ReplaceObjectsTile::swapObjectTile(MapObject *object, Tile *tile) {
    Cell cell = object->cell();
    cell.setTile(tile);
    object->setCell(cell);
    if (object->size() != tile->size())
        object->setSize(tile->size());
}

} // namespace Internal
} // namespace Tiled
