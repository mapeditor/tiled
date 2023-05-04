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
#include "geometry.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "tilelayer.h"

#include <QCoreApplication>

using namespace Tiled;

Eraser::Eraser(QObject *parent)
    : AbstractTileTool("EraserTool",
                       tr("Eraser"),
                       QIcon(QLatin1String(
                               ":images/22/stock-tool-eraser.png")),
                       QKeySequence(Qt::Key_E),
                       nullptr,
                       parent)
{
}

void Eraser::tilePositionChanged(QPoint tilePos)
{
    Q_UNUSED(tilePos)

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
        } else if (event->button() == Qt::RightButton && !(event->modifiers() & Qt::ControlModifier)) {
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

void Eraser::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mAllLayers = modifiers & Qt::ShiftModifier;
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

    QList<QPair<QRegion, TileLayer*>> erasedRegions;

    auto *eraseCommand = new PaintTileLayer(mapDocument());
    eraseCommand->setText(QCoreApplication::translate("Undo Commands", "Erase"));
    eraseCommand->setMergeable(continuation);

    auto eraseOnLayer = [&] (TileLayer *tileLayer) {
        if (!tileLayer->isUnlocked())
            return;

        QRegion eraseRegion = globalEraseRegion.intersected(tileLayer->bounds());
        if (eraseRegion.isEmpty())
            return;

        eraseCommand->erase(tileLayer, eraseRegion);

        erasedRegions.append({ eraseRegion, tileLayer });
    };

    if (mAllLayers) {
        for (Layer *layer : mapDocument()->map()->tileLayers())
            eraseOnLayer(static_cast<TileLayer*>(layer));
    } else if (!mapDocument()->selectedLayers().isEmpty()) {
        for (Layer *layer : mapDocument()->selectedLayers())
            if (TileLayer *tileLayer = layer->asTileLayer())
                eraseOnLayer(tileLayer);
    } else if (auto tileLayer = currentTileLayer()) {
        eraseOnLayer(tileLayer);
    }

    if (!erasedRegions.isEmpty())
        mapDocument()->undoStack()->push(eraseCommand);

    for (auto &[region, tileLayer] : std::as_const(erasedRegions)) {
        if (tileLayer->map() != mapDocument()->map())
            continue;

        emit mapDocument()->regionEdited(region, tileLayer);
    }
}

QRect Eraser::eraseArea() const
{
    if (mMode == RectangleErase) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QRect rect = QRect(mStart, tilePosition()).normalized();
        if (rect.width() == 0)
            rect.adjust(-1, 0, 1, 0);
        if (rect.height() == 0)
            rect.adjust(0, -1, 0, 1);
        return rect;
#else
        return QRect::span(mStart, tilePosition());
#endif
    }

    return QRect(tilePosition(), QSize(1, 1));
}

#include "moc_eraser.cpp"
