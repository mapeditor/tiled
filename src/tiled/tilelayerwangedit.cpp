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

#include "editablemap.h"
#include "editabletilelayer.h"
#include "maprenderer.h"
#include "scriptmanager.h"
#include "tilelayer.h"
#include "wangfiller.h"

#include <QCoreApplication>

namespace Tiled {

TileLayerWangEdit::TileLayerWangEdit(EditableTileLayer *tileLayer, EditableWangSet *wangSet, QObject *parent)
    : QObject(parent)
    , mTargetLayer(tileLayer)
    , mWangSet(wangSet)
    , mMap(tileLayer->map()->map()->parameters())
    , mRenderer(MapRenderer::create(&mMap))
    , mWangFiller(std::make_unique<WangFiller>(*wangSet->wangSet(),
                                               *mTargetLayer->tileLayer(),
                                               mRenderer.get()))
{
    mTargetLayer->mActiveWangEdits.append(this);

    // Avoid usage of this object when the WangSet is deleted (this actually
    // requires keeping the EditableWangSet alive).
    connect(mWangSet, &QObject::destroyed, this, &QObject::deleteLater);
}

TileLayerWangEdit::~TileLayerWangEdit()
{
    mTargetLayer->mActiveWangEdits.removeOne(this);
}

bool TileLayerWangEdit::correctionsEnabled() const
{
    return mWangFiller->correctionsEnabled();
}

void TileLayerWangEdit::setCorrectionsEnabled(bool correctionsEnabled)
{
    mWangFiller->setCorrectionsEnabled(correctionsEnabled);
}

bool TileLayerWangEdit::erasingEnabled() const
{
    return mWangFiller->erasingEnabled();
}

void TileLayerWangEdit::setErasingEnabled(bool erasingEnabled)
{
    mWangFiller->setErasingEnabled(erasingEnabled);
}

void TileLayerWangEdit::setWangIndex(QPoint pos, WangIndex::Value index, int color)
{
    mWangFiller->setWangIndex(pos, static_cast<WangId::Index>(index), color);
}

void TileLayerWangEdit::setCorner(QPoint pos, int color)
{
    mWangFiller->setCorner(pos, color);
}

void TileLayerWangEdit::setEdge(QPoint pos, WangIndex::Value edge, int color)
{
    switch (edge) {
    case WangIndex::Top:
    case WangIndex::Right:
    case WangIndex::Bottom:
    case WangIndex::Left:
        mWangFiller->setEdge(pos, static_cast<WangId::Index>(edge), color);
        break;
    default:
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid edge index"));
        break;
    }
}

EditableTileLayer *TileLayerWangEdit::generate()
{
    auto changes = std::make_unique<TileLayer>();
    mWangFiller->apply(*changes);
    return new EditableTileLayer(std::move(changes));
}

void TileLayerWangEdit::apply()
{
    // Applying an edit automatically makes it mergeable, so that further
    // changes made through the same edit are merged by default.
    const bool mergeable = std::exchange(mMergeable, true);

    // Apply terrain changes
    TileLayer changes;
    mWangFiller->apply(changes);
    mTargetLayer->applyChangesFrom(&changes, mergeable);
}

} // namespace Tiled

#include "moc_tilelayerwangedit.cpp"
