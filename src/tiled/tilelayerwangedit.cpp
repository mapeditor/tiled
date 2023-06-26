/*
 * tilelayerwangedit.cpp
 * Copyright 2023, a-morphous
 * Copyright 2023, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "editabletilelayer.h"
#include "mapdocument.h"
#include "scriptmanager.h"

#include <QCoreApplication>

namespace Tiled {

TileLayerWangEdit::TileLayerWangEdit(EditableTileLayer *tileLayer, EditableWangSet *wangSet, QObject *parent)
    : QObject(parent)
    , mTargetLayer(tileLayer)
{
    // todo: what if the WangSet is deleted?
    mTargetLayer->mActiveWangEdits.append(this);

    // todo: don't crash when given target layer without document
    mWangFiller = std::make_unique<WangFiller>(*wangSet->wangSet(),
                                               mTargetLayer->mapDocument()->renderer());
}

TileLayerWangEdit::~TileLayerWangEdit()
{
    mTargetLayer->mActiveWangEdits.removeOne(this);
}

void TileLayerWangEdit::setWangIndex(QPoint pos, WangIndex index, int color)
{
    mWangFiller->setWangIndex(pos, static_cast<WangId::Index>(index), color);
}

void TileLayerWangEdit::setCorner(QPoint pos, int color)
{
    mWangFiller->setCorner(pos, color);
}

void TileLayerWangEdit::setEdge(QPoint pos, WangIndex edge, int color)
{
    switch (edge) {
    case Top:
    case Right:
    case Bottom:
    case Left:
        mWangFiller->setEdge(pos, static_cast<WangId::Index>(edge), color);
        break;
    default:
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid edge index"));
        break;
    }
}

void TileLayerWangEdit::apply()
{
    // apply terrain changes
    mWangFiller->apply(mChanges, *mTargetLayer->tileLayer());

    // Applying an edit automatically makes it mergeable, so that further
    // changes made through the same edit are merged by default.
    bool mergeable = std::exchange(mMergeable, true);
    mTargetLayer->applyChangesFrom(&mChanges, mergeable);
    mChanges.clear();
}

} // namespace Tiled

#include "moc_tilelayerwangedit.cpp"
