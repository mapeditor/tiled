/*
 * pointhandle.h
 * Copyright 2011-2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QGraphicsItem>

namespace Tiled {

class MapObject;


/**
 * A handle that allows moving around a point of a polygon.
 */
class PointHandle : public QGraphicsItem
{
public:
    PointHandle(MapObject *mapObject, int pointIndex);

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

    MapObject *mapObject() const { return mMapObject; }

    int pointIndex() const { return mPointIndex; }

    // These hide the QGraphicsItem members
    void setSelected(bool selected);
    bool isSelected() const { return mSelected; }

    void setHighlighted(bool highlighted);
    bool isHighlighted() const { return mHighlighted; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    MapObject *mMapObject;
    int mPointIndex;
    bool mSelected;
    bool mHighlighted;
};

} // namespace Tiled
