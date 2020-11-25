/*
 * wangfiller.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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
#include "map.h"
#include "wangset.h"

#include <QList>
#include <QMap>
#include <QPoint>

#include <memory>

namespace Tiled {

class MapRenderer;
class StaggeredRenderer;

/**
 * WangFiller provides functions for choosing cells based on a surrounding map
 * and a wangSet.
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

    explicit WangFiller(const WangSet &wangSet, const MapRenderer *mapRenderer);

    void setErasingEnabled(bool enabled) { mErasingEnabled = enabled; }
    void setCorrectionsEnabled(bool enabled) { mCorrectionsEnabled = enabled; }

    void setDebugPainter(QPainter *painter) { mDebugPainter = painter; }

    /**
     * Fills the given \a region in the \a target layer with Wang methods,
     * based on the desired \a wangIds.
     *
     * The \a back layer is used to match up the edges to existing tiles.
     */
    void fillRegion(TileLayer &target,
                    const TileLayer &back,
                    const QRegion &region,
                    Grid<CellInfo> wangIds = {}) const;

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
    const StaggeredRenderer * const mStaggeredRenderer;
    bool mErasingEnabled = false;
    bool mCorrectionsEnabled = false;

    QPainter *mDebugPainter = nullptr;
};

} // namespace Tiled
