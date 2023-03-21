/*
 * objectreferenceitem.h
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

#include <QGraphicsItem>

namespace Tiled {

class MapObject;
class MapRenderer;

class ArrowHead;

class ObjectReferenceItem : public QGraphicsItem
{
public:
    ObjectReferenceItem(MapObject *source,
                        QGraphicsItem *parent = nullptr);

    ObjectReferenceItem(MapObject *source,
                        MapObject *target,
                        QGraphicsItem *parent = nullptr);

    MapObject *sourceObject() const { return mSourceObject; }
    MapObject *targetObject() const { return mTargetObject; }

    void setSourceObject(MapObject *sourceObject) { mSourceObject = sourceObject; }
    void setTargetPos(const QPointF &pos);
    void setTargetObject(MapObject *targetObject) { mTargetObject = targetObject; }

    void syncWithSourceObject(const MapRenderer &renderer);
    void syncWithTargetObject(const MapRenderer &renderer);
    void updateColor();

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

private:
    void updateArrowRotation();

    QPointF objectCenter(MapObject *object,
                         const MapRenderer &renderer) const;

    QPointF mSourcePos;
    QPointF mTargetPos;
    MapObject *mSourceObject;
    MapObject *mTargetObject;
    ArrowHead *mArrowHead;
    QColor mColor;
};

} // namespace Tiled
