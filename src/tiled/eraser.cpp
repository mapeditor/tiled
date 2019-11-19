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

Eraser::Eraser(QObject *parent)
    : AbstractTileTool("EraserTool",
                       tr("Eraser"),
                       QIcon(QLatin1String(
                               ":images/22/stock-tool-eraser.png")),
                       QKeySequence(Qt::Key_E),
                       nullptr,
                       parent)
    , mMode(Nothing)
{
}

void Eraser::tilePositionChanged(QPoint tilePos)
{
    Q_UNUSED(tilePos);

    brushItem()->setTileRegion(eraseArea());

    if (mMode == Erase)
        doErase(true);
}

void Eraser::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (brushItem()->isVisible() && mMode == Nothing) {
        if (event->button() == Qt::LeftButton) {
            mMode = Erase;
            doErase(false);
            return;
        } else if (event->button() == Qt::RightButton && event->modifiers() == Qt::NoModifier) {
            mStart = tilePosition();
            mMode = RectangleErase;
            return;
        }
    }

    AbstractTileTool::mousePressed(event);
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
}

void Eraser::doErase(bool continuation)
{
    QRegion globalEraseRegion(eraseArea());
    QPoint tilePos = tilePosition();

    if (continuation) {
        const QVector<QPoint> points = pointsOnLine(mLastTilePos, tilePos);
        for (const QPoint &p : points)
            globalEraseRegion |= QRegion(p.x(), p.y(), 1, 1);
    }
    mLastTilePos = tilePos;

    for (Layer *layer : mapDocument()->selectedLayers()) {
        if (!layer->isTileLayer())
            continue;
        if (!layer->isUnlocked())
            continue;

        auto tileLayer = static_cast<TileLayer*>(layer);

        QRegion eraseRegion = globalEraseRegion.intersected(tileLayer->bounds());
        if (eraseRegion.isEmpty())
            continue;

        EraseTiles *erase = new EraseTiles(mapDocument(), tileLayer, eraseRegion);
        erase->setMergeable(continuation);

        mapDocument()->undoStack()->push(erase);
        emit mapDocument()->regionEdited(eraseRegion, tileLayer);

        continuation = true;    // further erases are always continuations
    }
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
