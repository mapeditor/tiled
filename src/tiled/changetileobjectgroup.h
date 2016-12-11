#ifndef CHANGETILEOBJECTGROUP_H
#define CHANGETILEOBJECTGROUP_H

#include <QUndoCommand>

namespace Tiled {

class ObjectGroup;
class Tile;

namespace Internal {

class MapDocument;

class ChangeTileObjectGroup : public QUndoCommand
{
public:
    /**
     * Creates a command that changes the ObjectGroup of the given \a tile. The
     * command takes ownership of the \a objectGroup.
     */
    ChangeTileObjectGroup(MapDocument *mapDocument,
                          Tile *tile,
                          ObjectGroup *objectGroup,
                          QUndoCommand *parent = nullptr);

    ~ChangeTileObjectGroup();

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    MapDocument *mMapDocument;
    Tile *mTile;
    ObjectGroup *mObjectGroup;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGETILEOBJECTGROUP_H
