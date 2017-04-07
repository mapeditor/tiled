#ifndef REPLACESELECTEDOBJECTSTILE_H
#define REPLACESELECTEDOBJECTSTILE_H

#include "undocommands.h"

#include "mapobject.h"
#include <QList>
#include <QUndoCommand>

namespace Tiled {

class Tile;

namespace Internal {

class MapDocument;

class ReplaceSelectedObjectsTile : public QUndoCommand
{
public:
    ReplaceSelectedObjectsTile(MapDocument *mapDocument,
                              const QList<MapObject *> &mapObjects,
                              Tile *tile);

     void undo() { replace(); }
     void redo() { replace(); }

private:
    void replace();

    MapDocument *mMapDocument;
    const QList<MapObject *> mMapObjects;
    Tile *mTile;
};

} // namespace Internal
} // namespace Tiled

#endif // REPLACESELECTEDOBJECTSTILE_H
