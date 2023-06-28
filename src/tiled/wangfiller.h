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
        WangId desired;
        WangId mask;

        bool operator==(const CellInfo &other) const {
            return desired == other.desired && mask == other.mask;
        }
    };

    struct FillRegion {
        Grid<CellInfo> grid;
        QRegion region;
    };

    explicit WangFiller(const WangSet &wangSet, const MapRenderer *mapRenderer);

    FillRegion &region() { return mFillRegion; }

    bool correctionsEnabled() const { return mCorrectionsEnabled; }
    void setCorrectionsEnabled(bool enabled) { mCorrectionsEnabled = enabled; }

    void setDebugPainter(QPainter *painter) { mDebugPainter = painter; }

    void setRegion(const QRegion &region);
    void setWangIndex(QPoint pos, WangId::Index index, int color);
    void setCorner(QPoint vertexPos, int color);
    void setEdge(QPoint pos, WangId::Index index, int color);

    /**
     * Applies the scheduled Wang changes to the \a target layer.
     *
     * The \a back layer is used to match up the edges to existing tiles.
     */
    void apply(TileLayer &target, const TileLayer &back);

private:
    /**
     * Returns a wangId based on cells from \a back which are not in the
     * \a region. \a point and \a region are relative to \a back.
     */
    WangId wangIdFromSurroundings(const TileLayer &back,
                                  const QRegion &region,
                                  QPoint point) const;

    bool findBestMatch(const TileLayer &target,
                       const Grid<CellInfo> &grid,
                       QPoint position,
                       Cell &result) const;

    const WangSet &mWangSet;
    const MapRenderer * const mMapRenderer;
    const HexagonalRenderer * const mHexagonalRenderer;
    bool mCorrectionsEnabled = false;
    FillRegion mFillRegion;

    QPainter *mDebugPainter = nullptr;
};

} // namespace Tiled
