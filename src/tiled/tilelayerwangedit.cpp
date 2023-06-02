#include "tilelayerwangedit.h"

#include "addremovetileset.h"
#include "editablemap.h"
#include "editabletile.h"
#include "editabletilelayer.h"
#include "painttilelayer.h"
#include "scriptmanager.h"

namespace Tiled {

TileLayerWangEdit::TileLayerWangEdit(EditableTileLayer *tileLayer, EditableWangSet *wangSet, QObject *parent)
    : QObject(parent)
    , mTargetLayer(tileLayer)
{
    mTargetLayer->mActiveWangEdits.append(this);
	mWangPainter = new WangPainter();
	mWangPainter->setWangSet(wangSet->wangSet());
}

TileLayerWangEdit::~TileLayerWangEdit()
{
    mTargetLayer->mActiveWangEdits.removeOne(this);
}

void TileLayerWangEdit::setTerrain(int x, int y, int color, WangId::Index direction) {
    mWangPainter->setTerrain(mTargetLayer->mapDocument(), color, QPoint(x, y), direction);
}

void TileLayerWangEdit::apply()
{
	// apply terrain changes
    mWangPainter->commit(mTargetLayer->mapDocument(), &mChanges);

    // Applying an edit automatically makes it mergeable, so that further
    // changes made through the same edit are merged by default.
    bool mergeable = std::exchange(mMergeable, true);
	mTargetLayer->applyChangesFrom(&mChanges, mergeable);
    mChanges.clear();
}

} // namespace Tiled

#include "moc_tilelayerwangedit.cpp"
