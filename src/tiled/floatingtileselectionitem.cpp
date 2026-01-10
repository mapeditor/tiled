/*
 * floatingtileselectionitem.cpp
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

#include "floatingtileselectionitem.h"

#include "mapdocument.h"
#include "maprenderer.h"
#include "tilelayer.h"
#include "utils.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimerEvent>

#include <cmath>

using namespace Tiled;

FloatingTileSelectionItem::FloatingTileSelectionItem(MapDocument *mapDocument,
                                                     QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mMapDocument(mapDocument)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
}

FloatingTileSelectionItem::~FloatingTileSelectionItem()
{
    if (mAnimationTimer != -1)
        killTimer(mAnimationTimer);
}

void FloatingTileSelectionItem::setTiles(TileLayer *tileLayer, const QRegion &region)
{
    mTileLayer = tileLayer;
    mRegion = region;

    if (mTileLayer && !mRegion.isEmpty()) {
        if (mAnimationTimer == -1)
            mAnimationTimer = startTimer(AnimationInterval);
        show();
    } else {
        if (mAnimationTimer != -1) {
            killTimer(mAnimationTimer);
            mAnimationTimer = -1;
        }
        hide();
    }

    updateBoundingRect();
    update();
}

void FloatingTileSelectionItem::setTileOffset(QPoint offset)
{
    if (mTileOffset == offset)
        return;

    mTileOffset = offset;
    updateBoundingRect();
    update();
}

QRectF FloatingTileSelectionItem::boundingRect() const
{
    return mBoundingRect;
}

void FloatingTileSelectionItem::paint(QPainter *painter,
                                      const QStyleOptionGraphicsItem *option,
                                      QWidget *)
{
    if (!mTileLayer || mRegion.isEmpty())
        return;

    MapRenderer *mapRenderer = renderer();
    if (!mapRenderer)
        return;

    const qreal devicePixelRatio = painter->device()->devicePixelRatioF();
    const qreal dashLength = std::ceil(Utils::dpiScaled(2) * devicePixelRatio);

    painter->setOpacity(0.8);

    painter->save();

    QPointF pixelOffset = mapRenderer->tileToPixelCoords(QPointF(mTileOffset));
    painter->translate(pixelOffset);

    mapRenderer->drawTileLayer(painter, mTileLayer, option->exposedRect.translated(-pixelOffset));

    painter->restore();
    painter->setOpacity(1.0);

    QRegion offsetRegion = mRegion.translated(mTileOffset);

    for (const QRect &rect : offsetRegion) {
        QRectF screenRect = mapRenderer->boundingRect(rect);

        screenRect.adjust(0.5, 0.5, -0.5, -0.5);

        const QLineF lines[4] = {
            QLineF(screenRect.topLeft(), screenRect.topRight()),
            QLineF(screenRect.bottomLeft(), screenRect.bottomRight()),
            QLineF(screenRect.topLeft(), screenRect.bottomLeft()),
            QLineF(screenRect.topRight(), screenRect.bottomRight())
        };

        QPen pen(Qt::white, 1.5 * devicePixelRatio, Qt::SolidLine);
        pen.setCosmetic(true);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(pen);
        painter->drawLines(lines, 4);

        pen.setColor(Qt::black);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({dashLength, dashLength});
        pen.setDashOffset(mDashOffset);
        painter->setPen(pen);
        painter->drawLines(lines, 4);
    }
}

void FloatingTileSelectionItem::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mAnimationTimer) {
        mDashOffset++;
        update();
    } else {
        QGraphicsObject::timerEvent(event);
    }
}

void FloatingTileSelectionItem::updateBoundingRect()
{
    prepareGeometryChange();

    if (!mTileLayer || mRegion.isEmpty()) {
        mBoundingRect = QRectF();
        return;
    }

    MapRenderer *mapRenderer = renderer();
    if (!mapRenderer) {
        mBoundingRect = QRectF();
        return;
    }

    QRegion offsetRegion = mRegion.translated(mTileOffset);
    QRectF bounds;

    for (const QRect &rect : offsetRegion) {
        QRectF screenRect = mapRenderer->boundingRect(rect);
        if (bounds.isEmpty())
            bounds = screenRect;
        else
            bounds = bounds.united(screenRect);
    }

    mBoundingRect = bounds.adjusted(-2, -2, 2, 2);
}

MapRenderer *FloatingTileSelectionItem::renderer() const
{
    return mMapDocument ? mMapDocument->renderer() : nullptr;
}

#include "moc_floatingtileselectionitem.cpp"
