/*
 * debugdrawitem.h
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

#pragma once

#include "id.h"

#include <QElapsedTimer>
#include <QGraphicsItem>
#include <QHash>
#include <QPainter>
#include <QPicture>

namespace Tiled {

class MapScene;

class DebugDrawItem final : public QGraphicsItem
{
public:
    DebugDrawItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    friend class DebugDraw;

    QPicture &begin(Id id);
    void end(Id id);
    void clear(Id id);

    struct Entry {
        QElapsedTimer timer;
        QPicture picture;
    };

    QHash<Id, Entry> mEntries;

    mutable QRectF mBoundingRect;
    mutable bool mBoundingRectDirty = false;
};

class DebugDraw : public QPainter
{
public:
    DebugDraw(MapScene *scene, Id id);
    ~DebugDraw();

private:
    Id mId;
    DebugDrawItem *mDrawItem;
};

} // namespace Tiled
