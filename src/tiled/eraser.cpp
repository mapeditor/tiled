/*
 * eraser.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "eraser.h"

#include "brushitem.h"
#include "erasetiles.h"
#include "geometry.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

Eraser::Eraser(QObject *parent)
    : AbstractTileTool(tr("Eraser"),
                       QIcon(QLatin1String(":images/22x22/stock-tool-eraser.png")),
                       QKeySequence(tr("E")),
                       parent)
    , mMode(Nothing)
{
}

void Eraser::tilePositionChanged(const QPoint &tilePos)
{
    Q_UNUSED(tilePos);

    brushItem()->setTileRegion(eraseArea());

    if (mMode == Erase)
        doErase(true);
}

void Eraser::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (!brushItem()->isVisible())
        return;

    if (mMode == Nothing) {
        if (event->button() == Qt::LeftButton) {
            mMode = Erase;
            doErase(false);
        } else if (event->button() == Qt::RightButton) {
            mStart = tilePosition();
            mMode = RectangleErase;
        }
    }
}

void Eraser::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    switch (mMode) {
    case Nothing:
        break;
    case Erase:
        if (event->button() == Qt::LeftButton)
            mMode = Nothing;
        break;
    case RectangleErase:
        if (event->button() == Qt::RightButton) {
            doErase(false);
            mMode = Nothing;
            brushItem()->setTileRegion(eraseArea());
        }
        break;
    }
}

void Eraser::languageChanged()
{
    setName(tr("Eraser"));
    setShortcut(QKeySequence(tr("E")));
}

void Eraser::doErase(bool continuation)
{
    TileLayer *tileLayer = currentTileLayer();
    QRegion eraseRegion(eraseArea());

    if (continuation) {
        const QPoint tilePos = tilePosition();
        const QVector<QPoint> points = pointsOnLine(mLastTilePos, tilePos);
        for (const QPoint &p : points)
            eraseRegion |= QRegion(p.x(), p.y(), 1, 1);
    }
    mLastTilePos = tilePosition();

    if (!tileLayer->bounds().intersects(eraseRegion.boundingRect()))
        return;

    EraseTiles *erase = new EraseTiles(mapDocument(), tileLayer, eraseRegion);
    erase->setMergeable(continuation);

    mapDocument()->undoStack()->push(erase);
    emit mapDocument()->regionEdited(eraseRegion, tileLayer);
}

QRect Eraser::eraseArea() const
{
    if (mMode == RectangleErase) {
        QRect rect = QRect(mStart, tilePosition()).normalized();
        if (rect.width() == 0)
            rect.adjust(-1, 0, 1, 0);
        if (rect.height() == 0)
            rect.adjust(0, -1, 0, 1);
        return rect;
    }

    return QRect(tilePosition(), QSize(1, 1));
}
