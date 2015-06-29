/*
 * filltiles.cpp
 * Copyright 2009, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "filltiles.h"

#include "tilelayer.h"
#include "tilepainter.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

FillTiles::FillTiles(MapDocument *mapDocument,
                     TileLayer *tileLayer,
                     const QRegion &fillRegion,
                     const SharedTileLayer &fillStamp,
                     QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Fill Area"),
                   parent)
    , mMapDocument(mapDocument)
    , mTileLayer(tileLayer)
    , mFillRegion(fillRegion)
    , mOriginalCells(tileLayer->copy(mFillRegion))
    , mFillStamp(fillStamp)
{
}

FillTiles::~FillTiles()
{
    delete mOriginalCells;
}

void FillTiles::undo()
{
    const QRect boundingRect = mFillRegion.boundingRect();
    TilePainter painter(mMapDocument, mTileLayer);
    painter.setCells(boundingRect.x(),
                     boundingRect.y(),
                     mOriginalCells,
                     mFillRegion);

    QUndoCommand::undo(); // undo child commands
}

void FillTiles::redo()
{
    QUndoCommand::redo(); // redo child commands

    TilePainter painter(mMapDocument, mTileLayer);
    painter.drawStamp(mFillStamp.data(), mFillRegion);
}
