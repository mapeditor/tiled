/*
 * wangfiller.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 * Copyright 2020-2023, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "grid.h"
#include "wangset.h"

#include <QList>
#include <QMap>
#include <QPoint>

#include <memory>

namespace Tiled {

class MapRenderer;
class HexagonalRenderer;

/**
 * Provides functions for choosing cells based on a surrounding map and a
 * WangSet.
 *
 * Optionally when choosing cells, this will look at adjacent cells
 * to ensure that they will be able to be filled based on the chosen cell.
 */
class WangFiller
{
public:
    struct CellInfo {
        WangId desired = WangId::FULL_MASK;
        WangId mask;

        bool operator==(const CellInfo &other) const {
            return desired == other.desired && mask == other.mask;
        }
    };

    struct FillRegion {
        Grid<CellInfo> grid;
        QRegion region;
    };

    /**
     * Constructs a WangFiller that works with the given \a wangSet and uses
     * the \a back layer to match up the edges to existing tiles.
     */
    explicit WangFiller(const WangSet &wangSet,
                        const TileLayer &back,
                        const MapRenderer *mapRenderer);

    FillRegion &region() { return mFillRegion; }

    bool correctionsEnabled() const { return mCorrectionsEnabled; }
    void setCorrectionsEnabled(bool enabled) { mCorrectionsEnabled = enabled; }

    bool erasingEnabled() const { return mErasingEnabled; }
    void setErasingEnabled(bool enabled) { mErasingEnabled = enabled; }

    void setDebugPainter(QPainter *painter) { mDebugPainter = painter; }

    void setRegion(const QRegion &region);
    CellInfo &changePosition(QPoint pos);
    void setWangIndex(QPoint pos, WangId::Index index, int color);
    void setCorner(QPoint vertexPos, int color);
    void setEdge(QPoint pos, WangId::Index index, int color);

    void apply(TileLayer &target);

    /**
     * Returns the region with locations for which a matching tile could not be
     * found in the last call to apply().
     */
    const QRegion &invalidRegion() const { return mInvalidRegion; }

private:
    /**
     * Returns a wangId based the cells surrounding the given point, which
     * are outside of the current region.
     */
    WangId wangIdFromSurroundings(QPoint point) const;
    WangId wangIdFromSurroundingCells(const Cell surroundingCells[]) const;

    bool findBestMatch(const TileLayer &target,
                       const Grid<CellInfo> &grid,
                       QPoint position,
                       Cell &result) const;

    const WangSet &mWangSet;
    const TileLayer &mBack;
    const MapRenderer * const mMapRenderer;
    const HexagonalRenderer * const mHexagonalRenderer;
    bool mCorrectionsEnabled = false;
    bool mErasingEnabled = true;
    FillRegion mFillRegion;
    QRegion mInvalidRegion;

    QPainter *mDebugPainter = nullptr;
};

} // namespace Tiled
