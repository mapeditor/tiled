#include "changetilewangid.h"

#include "tilesetdocument.h"
#include "tile.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Internal;

ChangeTileWangId::ChangeTileWangId()
    : mTilesetDocument(nullptr)
    , mWangSet(nullptr)
    , mMergeable(false)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Tile WangId"));
}

ChangeTileWangId::ChangeTileWangId(TilesetDocument *tilesetDocument, WangSet *wangSet, Tile *tile, WangId wangId)
    : mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mMergeable(true)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Tile WangId"));
    if (mWangSet)
        mChanges.append(WangIdChange(mWangSet->wangIdOfTile(tile), wangId, tile));
}

ChangeTileWangId::ChangeTileWangId(TilesetDocument *tilesetDocument,
                                   WangSet *wangSet,
                                   const QList<WangIdChange> &changes,
                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mChanges(changes)
    , mMergeable(true)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Tile WangId"));
}

void ChangeTileWangId::undo()
{
    if (mChanges.isEmpty())
        return;

    QList<Tile *> changedTiles;

    for (WangIdChange wangIdChange : mChanges) {
        changedTiles.append(wangIdChange.tile);

        mWangSet->addTile(wangIdChange.tile, wangIdChange.from);
    }

    emit mTilesetDocument->tileWangSetChanged(changedTiles);
}

void ChangeTileWangId::redo()
{
    if (mChanges.isEmpty())
        return;

    QList<Tile *> changedTiles;

    for (WangIdChange &wangIdChange : mChanges) {
        changedTiles.append(wangIdChange.tile);

        mWangSet->addTile(wangIdChange.tile, wangIdChange.to);
    }

    emit mTilesetDocument->tileWangSetChanged(changedTiles);
}

bool ChangeTileWangId::mergeWith(const QUndoCommand *other)
{
    if (!mMergeable)
        return false;

    const ChangeTileWangId *o = static_cast<const ChangeTileWangId*>(other);
    if (o->mTilesetDocument && !(mTilesetDocument == o->mTilesetDocument &&
                                 mWangSet == o->mWangSet))
        return false;

    //suboptimal, could use a map to remove any unnessesary changes if the same tile has
    //multiple changes.
    mChanges.append(o->mChanges);

    mMergeable = o->mMergeable;

    return true;
}
