/*
 * movetiles.cpp
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

#include "movetiles.h"

#include "mapdocument.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilepainter.h"

#include <QCoreApplication>

using namespace Tiled;

MoveTiles::MoveTiles(MapDocument *mapDocument,
                     TileLayer *layer,
                     const QRegion &sourceRegion,
                     QPoint offset,
                     bool duplicate,
                     QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mLayer(layer)
    , mSourceRegion(sourceRegion)
    , mOffset(offset)
    , mDuplicate(duplicate)
{
    if (duplicate)
        setText(QCoreApplication::translate("Undo Commands", "Duplicate Tiles"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Move Tiles"));

    mDestRegion = sourceRegion.translated(offset);

    mMovedTiles = std::make_unique<TileLayer>();
    mMovedTiles->setCells(layer->x(), layer->y(), layer, sourceRegion);

    mOriginalSourceTiles = std::make_unique<TileLayer>();
    mOriginalSourceTiles->setCells(layer->x(), layer->y(), layer, sourceRegion);

    mOriginalDestTiles = std::make_unique<TileLayer>();
    mOriginalDestTiles->setCells(layer->x(), layer->y(), layer, mDestRegion);
}

MoveTiles::~MoveTiles()
{
}

void MoveTiles::undo()
{
    for (const QRect &rect : mDestRegion) {
        for (int y = rect.top(); y <= rect.bottom(); ++y) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                const Cell &cell = mOriginalDestTiles->cellAt(x, y);
                mLayer->setCell(x, y, cell);
            }
        }
    }

    if (!mDuplicate) {
        for (const QRect &rect : mSourceRegion) {
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                for (int x = rect.left(); x <= rect.right(); ++x) {
                    const Cell &cell = mOriginalSourceTiles->cellAt(x, y);
                    mLayer->setCell(x, y, cell);
                }
            }
        }
        emit mMapDocument->regionChanged(mSourceRegion, mLayer);
    }

    emit mMapDocument->regionChanged(mDestRegion, mLayer);
}

void MoveTiles::redo()
{
    if (!mDuplicate) {
        TilePainter painter(mMapDocument, mLayer);
        painter.erase(mSourceRegion);
    }

    for (const QRect &rect : mSourceRegion) {
        for (int y = rect.top(); y <= rect.bottom(); ++y) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                const Cell &cell = mMovedTiles->cellAt(x, y);
                if (!cell.isEmpty()) {
                    mLayer->setCell(x + mOffset.x(), y + mOffset.y(), cell);
                }
            }
        }
    }

    emit mMapDocument->regionChanged(mDestRegion, mLayer);
}
