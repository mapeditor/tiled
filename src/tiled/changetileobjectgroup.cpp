#include "changetileobjectgroup.h"

#include "mapdocument.h"
#include "objectgroup.h"
#include "tile.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

ChangeTileObjectGroup::ChangeTileObjectGroup(MapDocument *mapDocument,
                                             Tile *tile,
                                             ObjectGroup *objectGroup)
    : QUndoCommand(QCoreApplication::translate(
                       "Undo Commands", "Change Tile Collision"))
    , mMapDocument(mapDocument)
    , mTile(tile)
    , mObjectGroup(objectGroup)
{
}

ChangeTileObjectGroup::~ChangeTileObjectGroup()
{
    delete mObjectGroup;
}

void ChangeTileObjectGroup::swap()
{
    mObjectGroup = mTile->swapObjectGroup(mObjectGroup);
    mMapDocument->emitTileObjectGroupChanged(mTile);
}

} // namespace Internal
} // namespace Tiled
