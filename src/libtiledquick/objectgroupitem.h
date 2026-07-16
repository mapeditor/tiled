/*
 * objectgroupitem.h
 * Copyright 2026, UltraDagon
 *
 * This file is part of Tiled Quick.
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

#include <QQuickItem>

#include "objectgroup.h"
#include "tilelayer.h"
#include "tiledquick_global.h"


namespace Tiled {
class MapRenderer;
}

namespace TiledQuick {

class MapItem;

class TILEDQUICK_SHARED_EXPORT ObjectGroupItem : public QQuickItem
{
    Q_OBJECT
public:
    ObjectGroupItem(Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer,
                    MapItem *parent);

    void syncWithObjectGroup();

    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

    Tiled::ObjectGroup *group();

public slots:
    void updateVisibleObjects();

private:
    void groupVisibilityChanged();

    Tiled::ObjectGroup *mGroup;
    Tiled::MapRenderer *mRenderer;
    QRectF mVisibleArea;
};

class ObjectItem : public QQuickItem
{
    Q_OBJECT

public:
    ObjectItem(const Tiled::Cell &cell, QPoint position, MapItem *parent);

    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

private:
    Tiled::Cell mCell;
    QPoint mPosition;
};

inline Tiled::ObjectGroup *ObjectGroupItem::group()
{
    return mGroup;
}

} // namespace TiledQuick
