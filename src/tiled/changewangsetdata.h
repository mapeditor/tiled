#pragma once

#include <QUndoCommand>

namespace Tiled {

class Tileset;

namespace Internal {

class TilesetDocument;
class TilesetWangSetModel;

class ChangeWangSetEdges : public QUndoCommand
{
public:
    ChangeWangSetEdges(TilesetDocument *TilesetDocument,
                       int index,
                       int newValue);

    void undo() override;
    void redo() override;

private:
    TilesetWangSetModel *mWangSetModel;
    Tileset *mTileset;
    int mIndex;
    int mOldValue;
    int mNewValue;
};

class ChangeWangSetCorners : public QUndoCommand
{
public:
    ChangeWangSetCorners(TilesetDocument *TilesetDocument,
                       int index,
                       int newValue);

    void undo() override;
    void redo() override;

private:
    TilesetWangSetModel *mWangSetModel;
    Tileset *mTileset;
    int mIndex;
    int mOldValue;
    int mNewValue;
};

} // namespace Internal
} // namespace Tiled
