/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "painttilelayer.h"

#include "map.h"
#include "mapdocument.h"
#include "tilelayer.h"
#include "tilepainter.h"

using namespace Tiled;
using namespace Tiled::Internal;

PaintTileLayer::PaintTileLayer(MapDocument *mapDocument,
                               TileLayer *target,
                               int x,
                               int y,
                               const TileLayer *source):
    mMapDocument(mapDocument),
    mTarget(target),
    mSource(static_cast<TileLayer*>(source->clone())),
    mX(x),
    mY(y)
{
    mErased = mTarget->copy(mX - mTarget->x(),
                            mY - mTarget->y(),
                            mSource->width(), mSource->height());
    setText(QObject::tr("Paint"));
}

PaintTileLayer::~PaintTileLayer()
{
    delete mSource;
    delete mErased;
}

void PaintTileLayer::undo()
{
    TilePainter painter(mMapDocument, mTarget);
    painter.setTiles(mX, mY, mErased);
}

void PaintTileLayer::redo()
{
    TilePainter painter(mMapDocument, mTarget);
    painter.drawTiles(mX, mY, mSource);
}
