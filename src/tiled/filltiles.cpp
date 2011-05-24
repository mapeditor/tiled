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
                     const TileLayer *fillStamp)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Fill Area"))
    , mMapDocument(mapDocument)
    , mTileLayer(tileLayer)
    , mFillRegion(fillRegion)
    , mOriginalCells(tileLayer->copy(mFillRegion))
    , mFillStamp(static_cast<TileLayer*>(fillStamp->clone()))
{
}

FillTiles::~FillTiles()
{
    delete mOriginalCells;
    delete mFillStamp;
}

void FillTiles::undo()
{
    const QRect boundingRect = mFillRegion.boundingRect();
    TilePainter painter(mMapDocument, mTileLayer);
    painter.setCells(boundingRect.x(),
                     boundingRect.y(),
                     mOriginalCells,
                     mFillRegion);
}

void FillTiles::redo()
{
    TilePainter painter(mMapDocument, mTileLayer);
    painter.drawStamp(mFillStamp, mFillRegion);
}
