/*
 * debugdrawitem.cpp
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "debugdrawitem.h"

#include "mapscene.h"

#include <QDebug>
#include <QPainter>

namespace Tiled {

// TODO: Cleanup of entries after a certain time?

DebugDrawItem::DebugDrawItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setZValue(100000);
    setVisible(false);
}

QRectF DebugDrawItem::boundingRect() const
{
    if (mBoundingRectDirty) {
        mBoundingRect = QRectF();
        for (const Entry &entry : mEntries)
            mBoundingRect |= entry.picture.boundingRect();
    }

    return mBoundingRect;
}

void DebugDrawItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    for (const Entry &entry : std::as_const(mEntries))
        const_cast<QPicture *>(&entry.picture)->play(painter);
}

QPicture &DebugDrawItem::begin(Id id)
{
    auto &entry = mEntries[id];

    if (!entry.picture.isNull())
        update(entry.picture.boundingRect());

    entry.timer.start();
    entry.picture = QPicture();

    return entry.picture;
}

void DebugDrawItem::end(Id id)
{
    prepareGeometryChange();

    auto &entry = mEntries[id];
    auto rect = entry.picture.boundingRect();

    mBoundingRect |= rect;

    setVisible(true);
    update(rect);
}

void DebugDrawItem::clear(Id id)
{
    mEntries.remove(id);
    mBoundingRectDirty = true;
    setVisible(!mEntries.isEmpty());
}


DebugDraw::DebugDraw(MapScene *scene, Id id)
    : mId(id)
    , mDrawItem(scene->debugDrawItem())
{
    if (mDrawItem)
        begin(&mDrawItem->begin(id));
}

DebugDraw::~DebugDraw()
{
    if (mDrawItem) {
        end();
        mDrawItem->end(mId);
    }
}

} // namespace Tiled
