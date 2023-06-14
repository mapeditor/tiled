/*
 * wangpainter.h
 * Copyright 2023, a-morphous
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

#pragma once

#include "mapdocument.h"
#include "tilelayer.h"
#include "wangfiller.h"
#include "wangset.h"

namespace Tiled {

class WangPainter
{
public:
    enum BrushMode {
        PaintCorner,
        PaintEdge,
        PaintEdgeAndCorner,
        Idle // no valid color selected
    };

    WangPainter();
    virtual ~WangPainter();

    BrushMode brushMode();

    void setWangSet(const WangSet *wangSet);
    void setTerrain(WangFiller::FillRegion &fill, MapDocument *mapDocument, int color, QPoint pos, WangId::Index directionToGenerate, bool useTileMode = false);
    void setTerrain(MapDocument *mapDocument, int color, QPoint pos, WangId::Index directionToGenerate, bool useTileMode = false);
    void clear();
    void commit(MapDocument *mapDocument, TileLayer *tileLayer);

private:
    void setColor(int color);
    WangId::Index getDesiredDirection(WangId::Index initialDirection);
    void generateTerrainAt(MapDocument *mapDocument, WangFiller::FillRegion &fill, int color, QPoint pos, WangId::Index direction, bool useTileMode = false);

    const WangSet *mWangSet;

    int mCurrentColor = 0;
    WangFiller::FillRegion mCurrentFill;
    BrushMode mBrushMode = BrushMode::Idle;
};

} // namespace Tiled
