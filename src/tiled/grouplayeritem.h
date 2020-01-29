/*
 * grouplayeritem.h
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "layeritem.h"

#include "grouplayer.h"

namespace Tiled {

class GroupLayerItem : public LayerItem
{
public:
    GroupLayerItem(GroupLayer *groupLayer, QGraphicsItem *parent = nullptr);

    GroupLayer *groupLayer() const;

    // QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
};

inline GroupLayer *GroupLayerItem::groupLayer() const
{
    return static_cast<GroupLayer*>(layer());
}

} // namespace Tiled
