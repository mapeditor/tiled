/*
 * tilehighlightitem.h
 * Copyright 2026, PoonamMehan <poonammehan655@gmail.com>
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

#include <QGraphicsObject>
#include <QPointer>

namespace Tiled {

class MapDocument;

class TileHighlightItem : public QGraphicsObject
{
    Q_OBJECT

public:
    TileHighlightItem(MapDocument *mapDocument,
                      int tileX, int tileY,
                      QGraphicsItem *parent = nullptr);

    void startBlink();

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    void updatePosition();

    QPointer<MapDocument> mMapDocument;
    int mTileX;
    int mTileY;
    QRectF mBoundingRect;
};

} // namespace Tiled
