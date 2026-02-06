/*
 * floatingtileselectionitem.h
 * Copyright 2025, Tiled contributors
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

#include "tilelayer.h"

#include <QGraphicsObject>
#include <QRegion>

namespace Tiled {

class MapDocument;
class MapRenderer;

class FloatingTileSelectionItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit FloatingTileSelectionItem(MapDocument *mapDocument,
                                       QGraphicsItem *parent = nullptr);
    ~FloatingTileSelectionItem() override;

    void setTiles(TileLayer *tileLayer, const QRegion &region);
    void setTileOffset(QPoint offset);
    QPoint tileOffset() const { return mTileOffset; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void updateBoundingRect();
    MapRenderer *renderer() const;

    MapDocument *mMapDocument;
    TileLayer *mTileLayer = nullptr;
    QRegion mRegion;
    QPoint mTileOffset;
    QRectF mBoundingRect;

    int mAnimationTimer = -1;
    int mDashOffset = 0;
    static constexpr int AnimationInterval = 100;
};

} // namespace Tiled
