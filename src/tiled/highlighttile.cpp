/*
 * highlighttile.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017 Leon Moctezuma <leon.moctezuma@gmail.com>
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

#include "highlighttile.h"

#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "painttilelayer.h"
#include "tile.h"
#include "tilelayer.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

HighlightTile::HighlightTile():
    mMapDocument(nullptr),
    mAnimation(this, "color")
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);

    setColor(QApplication::palette().highlight().color());

    mAnimation.setStartValue(color());
    mAnimation.setEndValue(QColor(255,255,255,0));
    mAnimation.setDuration(1000);
    mAnimation.setEasingCurve(QEasingCurve::InOutCirc);
    mAnimation.setLoopCount(1);

}

/**
 * Sets the map document this brush is operating on.
 */
void HighlightTile::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;
}

/**
 * Sets the region of tiles that this highlight occupies.
 */
void HighlightTile::setTileRegion(const QRegion &region)
{
    if (mRegion == region)
        return;

    mRegion = region;
    updateBoundingRect();
}

QRectF HighlightTile::boundingRect() const
{
    return mBoundingRect;
}

void HighlightTile::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *)
{
    QColor highlightColor = color();
    //insideMapHighlight.setAlpha(64);
    
    int mapWidth = mMapDocument->map()->width();
    int mapHeight = mMapDocument->map()->height();
    QRegion mapRegion = QRegion(0, 0, mapWidth, mapHeight);

    const MapRenderer *renderer = mMapDocument->renderer();

    renderer->drawTileSelection(painter, mapRegion,
                                highlightColor,
                                option->exposedRect);
}

void HighlightTile::updateBoundingRect()
{
    prepareGeometryChange();

    if (!mMapDocument) {
        mBoundingRect = QRectF();
        return;
    }

    const QRect bounds = mRegion.boundingRect();
    mBoundingRect = mMapDocument->renderer()->boundingRect(bounds);
}

void HighlightTile::setColor(const QColor& color) {
    mColor = color;
    this->update();
}

void HighlightTile::updateHighlight() {

}

void HighlightTile::animate() {
    mAnimation.stop();
    mAnimation.start();
}
