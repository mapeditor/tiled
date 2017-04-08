#include "replaceselectedobjectstile.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"
#include "objectgroup.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <iostream>

namespace Tiled {
namespace Internal {

ReplaceSelectedObjectsTile::ReplaceSelectedObjectsTile(MapDocument *mapDocument,
                                                       const QList<MapObject *> &mapObjects,
                                                       Tile *tile)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Replace Selected Object/s Tile"))
    , mMapDocument(mapDocument)
    , mMapObjects(mapObjects)
    , mTile(tile)
{}

void ReplaceSelectedObjectsTile::replace()
{
    Tile * const tile = mTile;

    mOldTiles.clear();

    auto swapObjectTile = [=](MapObject *object, Tile *tile) {
        Cell cell = object->cell();
        cell.setTile(tile);
        object->setCell(cell);
        if (object->size() != tile->size())
            object->setSize(tile->size());
    };

    for (MapObject *object : mMapObjects) {
        mOldTiles.append(object->cell().tile());
        swapObjectTile(object, tile);
    }

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}

void ReplaceSelectedObjectsTile::restore()
{
    auto swapObjectTile = [=](MapObject *object, Tile *tile) {
        Cell cell = object->cell();
        cell.setTile(tile);
        object->setCell(cell);
        if (object->size() != tile->size())
            object->setSize(tile->size());
    };

    for(int i=0; i<mMapObjects.size(); i++) {
        swapObjectTile(mMapObjects[i], mOldTiles[i]);
    }

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);

}

} // namespace Internal
} // namespace Tiled
