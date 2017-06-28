#include "changewangsetdata.h"

#include "wangset.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetwangsetmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Internal;

ChangeWangSetEdges::ChangeWangSetEdges(TilesetDocument *tilesetDocument,
                                       int index,
                                       int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set edge count"))
    , mWangSetModel(tilesetDocument->wangSetModel())
    , mTileset(tilesetDocument->tileset().data())
    , mIndex(index)
    , mOldValue(mTileset->wangSet(index)->edgeColors())
    , mNewValue(newValue)
{
}

void ChangeWangSetEdges::undo()
{
    mWangSetModel->setWangSetEdges(mIndex, mOldValue);
}

void ChangeWangSetEdges::redo()
{
    mWangSetModel->setWangSetEdges(mIndex, mNewValue);
}

ChangeWangSetCorners::ChangeWangSetCorners(TilesetDocument *tilesetDocument,
                                       int index,
                                       int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set corner count"))
    , mWangSetModel(tilesetDocument->wangSetModel())
    , mTileset(tilesetDocument->tileset().data())
    , mIndex(index)
    , mOldValue(mTileset->wangSet(index)->cornerColors())
    , mNewValue(newValue)
{
}

void ChangeWangSetCorners::undo()
{
    mWangSetModel->setWangSetCorners(mIndex, mOldValue);
}

void ChangeWangSetCorners::redo()
{
    mWangSetModel->setWangSetCorners(mIndex, mNewValue);
}
