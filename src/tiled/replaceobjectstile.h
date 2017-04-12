#ifndef REPLACEOBJECTSTILE_H
#define REPLACEOBJECTSTILE_H

#include "undocommands.h"
#include "mapobject.h"

#include <QList>
#include <QUndoCommand>

namespace Tiled {

class Tile;

namespace Internal {

class MapDocument;

class ReplaceObjectsTile : public QUndoCommand
{
public:
    ReplaceObjectsTile(MapDocument *mapDocument,
                       const QList<MapObject *> &mapObjects,
                       Tile *tile);

    void undo() { restore(); }
    void redo() { replace(); }

private:
    void replace();
    void restore();

    MapDocument *mMapDocument;
    const QList<MapObject *> mMapObjects;
    QList<Tile *> mOriginalTiles;
    Tile *mTile;
};

} // namespace Internal
} // namespace Tiled

#endif // REPLACEOBJECTSTILE_H
