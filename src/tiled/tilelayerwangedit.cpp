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

// currently copied from tilelayeredit.cpp
void TileLayerWangEdit::apply()
{
	// apply terrain changes
    mWangPainter->commit(mTargetLayer->mapDocument(), &mChanges);

    // Applying an edit automatically makes it mergeable, so that further
    // changes made through the same edit are merged by default.
    bool mergeable = std::exchange(mMergeable, true);

    // Determine painted region and normalize the changes layer
    auto paintedRegion = mChanges.region([] (const Cell &cell) { return cell.checked(); });

    // If the painted region is empty there's nothing else to do
    if (paintedRegion.isEmpty())
        return;

    auto rect = paintedRegion.boundingRect();
    mChanges.resize(rect.size(), -rect.topLeft());
    const auto tilesets = mChanges.usedTilesets();

    if (mTargetLayer->mapDocument()) {
        // Apply the change using an undo command
        auto mapDocument = mTargetLayer->map()->mapDocument();
        auto paint = new PaintTileLayer(mapDocument,
                                        mTargetLayer->tileLayer(),
                                        rect.x(), rect.y(),
                                        &mChanges,
                                        paintedRegion);
        paint->setMergeable(mergeable);

        // Add any used tilesets that aren't yet part of the target map
        const auto existingTilesets = mapDocument->map()->tilesets();
        for (const SharedTileset &tileset : tilesets)
            if (!existingTilesets.contains(tileset))
                new AddTileset(mapDocument, tileset, paint);

        mTargetLayer->map()->push(paint);
    } else {
        // Add any used tilesets that aren't yet part of the target map
        if (auto map = mTargetLayer->tileLayer()->map())
            map->addTilesets(tilesets);

        // Apply the change directly
        mTargetLayer->tileLayer()->setCells(rect.x(), rect.y(), &mChanges, paintedRegion);
    }

    mChanges.clear();
}

} // namespace Tiled

#include "moc_tilelayerwangedit.cpp"
