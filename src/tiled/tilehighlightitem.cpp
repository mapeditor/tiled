/*
 * tilehighlightitem.cpp
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

#include "tilehighlightitem.h"

#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QStyleOptionGraphicsItem>

using namespace Tiled;

TileHighlightItem::TileHighlightItem(MapDocument *mapDocument,
                                     int tileX, int tileY,
                                     QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mMapDocument(mapDocument)
    , mTileX(tileX)
    , mTileY(tileY)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);

    const QRect b(tileX, tileY, 1, 1);
    mBoundingRect = mapDocument->renderer()->boundingRect(b);
    mBoundingRect.adjust(-1, -1, 1, 1);
}

void TileHighlightItem::startBlink()
{
    updatePosition();

    constexpr int blinkDuration = 120;
    constexpr int blinkCount = 3;

    auto *group = new QSequentialAnimationGroup(this);
    for (int i = 0; i < blinkCount; ++i) {
        auto *toVisible = new QPropertyAnimation(this, "opacity");
        toVisible->setDuration(blinkDuration);
        toVisible->setStartValue(0.0);
        toVisible->setEndValue(1.0);
        group->addAnimation(toVisible);

        auto *toHidden = new QPropertyAnimation(this, "opacity");
        toHidden->setDuration(blinkDuration);
        toHidden->setStartValue(1.0);
        toHidden->setEndValue(0.0);
        group->addAnimation(toHidden);
    }

    setOpacity(0.0);
    connect(group, &QAbstractAnimation::finished, this, &QObject::deleteLater);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void TileHighlightItem::updatePosition()
{
    if (!mMapDocument)
        return;
    if (auto currentLayer = mMapDocument->currentLayer())
        if (auto mapScene = qobject_cast<MapScene*>(scene()))
            setPos(mapScene->absolutePositionForLayer(*currentLayer));
}

QRectF TileHighlightItem::boundingRect() const
{
    return mBoundingRect;
}

void TileHighlightItem::paint(QPainter *painter,
                              const QStyleOptionGraphicsItem *option,
                              QWidget *)
{
    if (!mMapDocument) {
        deleteLater();
        return;
    }
    const QRegion region(QRect(mTileX, mTileY, 1, 1));
    QColor highlight = QApplication::palette().highlight().color();
    highlight.setAlpha(128);

    MapRenderer *renderer = mMapDocument->renderer();
    renderer->drawTileSelection(painter, region, highlight,
                                option->exposedRect);
}

#include "moc_tilehighlightitem.cpp"
