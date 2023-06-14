/*
 * tilelayerwangedit.cpp
 * Copyright 2023, a-morphous
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
    // todo: what if the WangSet is deleted?
    mTargetLayer->mActiveWangEdits.append(this);
	mWangPainter = new WangPainter();
	mWangPainter->setWangSet(wangSet->wangSet());
}

TileLayerWangEdit::~TileLayerWangEdit()
{
    mTargetLayer->mActiveWangEdits.removeOne(this);
}

void TileLayerWangEdit::setTerrain(int x, int y, int color, WangId::Index direction)
{
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
