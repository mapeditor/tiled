/*
 * wangfiller.cpp
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

#include "wangfiller.h"

#include "grid.h"
#include "randompicker.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"
#include "wangset.h"

using namespace Tiled;

static const QPoint aroundTilePoints[] = {
    QPoint( 0, -1),
    QPoint( 1, -1),
    QPoint( 1,  0),
    QPoint( 1,  1),
    QPoint( 0,  1),
    QPoint(-1,  1),
    QPoint(-1,  0),
    QPoint(-1, -1)
};

WangFiller::WangFiller(const WangSet &wangSet,
                       const MapRenderer *mapRenderer)
    : mWangSet(wangSet)
    , mMapRenderer(mapRenderer)
    , mStaggeredRenderer(dynamic_cast<const StaggeredRenderer*>(mapRenderer))
{
}

static void getSurroundingPoints(QPoint point,
                                 const StaggeredRenderer *staggeredRenderer,
                                 QPoint *points)
{
    if (staggeredRenderer) {
        points[0] = staggeredRenderer->topRight(point.x(), point.y());
        points[2] = staggeredRenderer->bottomRight(point.x(), point.y());
        points[4] = staggeredRenderer->bottomLeft(point.x(), point.y());
        points[6] = staggeredRenderer->topLeft(point.x(), point.y());

        if (staggeredRenderer->map()->staggerAxis() == Map::StaggerX) {
            points[1] = point + QPoint(2, 0);
            points[3] = point + QPoint(0, 1);
            points[5] = point + QPoint(-2, 0);
            points[7] = point + QPoint(0, -1);
        } else {
            points[1] = point + QPoint(1, 0);
            points[3] = point + QPoint(0, 2);
            points[5] = point + QPoint(-1, 0);
            points[7] = point + QPoint(0, -2);
        }
    } else {
        for (int i = 0; i < 8; ++i)
            points[i] = point + aroundTilePoints[i];
    }
}

/**
 * Matches the given \a info's edges/corners at \a position with an \a adjacent one.
 * Also sets the mask for the given corner / side.
 */
static void updateToAdjacent(WangFiller::CellInfo &info, WangId adjacent, int position)
{
    const int adjacentPosition = WangId::oppositeIndex(position);

    info.desired.setIndexColor(position, adjacent.indexColor(adjacentPosition));
    info.mask.setIndexColor(position, 0xf);

    const bool isCorner = position & 1;
    if (!isCorner) {
        const int cornerA = WangId::nextIndex(position);
        const int cornerB = WangId::previousIndex(position);
        const int adjacentCornerA = WangId::previousIndex(adjacentPosition);
        const int adjacentCornerB = WangId::nextIndex(adjacentPosition);

        info.desired.setIndexColor(cornerA, adjacent.indexColor(adjacentCornerA));
        info.mask.setIndexColor(cornerA, 0xf);

        info.desired.setIndexColor(cornerB, adjacent.indexColor(adjacentCornerB));
        info.mask.setIndexColor(cornerB, 0xf);
    }
}

void WangFiller::fillRegion(TileLayer &target,
                            const TileLayer &back,
                            const QRegion &region,
                            Grid<CellInfo> grid) const
{
    // Set the Wang IDs at the border of the region to make sure the tiles in
    // the filled region connect with those outside of it.
    auto setDesiredWangId = [&] (int x, int y) {
        const WangId source = wangIdFromSurroundings(back, region, QPoint(x, y));
        CellInfo info = grid.get(x, y);
        for (int i = 0; i < WangId::NumIndexes; ++i) {
            if (!info.mask.indexColor(i))
                if (int color = source.indexColor(i))
                    info.desired.setIndexColor(i, color);
        }
        grid.set(x, y, info);
    };

#if QT_VERSION < 0x050800
    const auto rects = region.rects();
    for (const QRect &rect : rects) {
#else
    for (const QRect &rect : region) {
#endif
        for (int x = rect.left(); x <= rect.right(); ++x) {
            setDesiredWangId(x, rect.top());
            setDesiredWangId(x, rect.bottom());
        }
        for (int y = rect.top() + 1; y < rect.bottom(); ++y) {
            setDesiredWangId(rect.left(), y);
            setDesiredWangId(rect.right(), y);
        }
    }

#if QT_VERSION < 0x050800
    for (const QRect &rect : rects) {
#else
    for (const QRect &rect : region) {
#endif
        for (int y = rect.top(); y <= rect.bottom(); ++y) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                WangTile wangTile;
                if (!findBestMatch(mWangSet, grid.get(x, y), wangTile)) {
                    // TODO: error feedback
                    continue;
                }

                auto cell = wangTile.makeCell();
                cell.setChecked(true);

                target.setCell(x - target.x(),
                               y - target.y(),
                               cell);

                // Adjust the desired WangIds for the surrounding tiles based on the placed one
                QPoint adjacentPoints[8];
                getSurroundingPoints(QPoint(x, y), mStaggeredRenderer, adjacentPoints);

                for (int i = 0; i < 8; ++i) {
                    const QPoint p = adjacentPoints[i];
                    if (target.cellAt(p - target.position()).checked())
                        continue;

                    CellInfo adjacentInfo = grid.get(p);
                    updateToAdjacent(adjacentInfo, wangTile.wangId(), WangId::oppositeIndex(i));
                    grid.set(p, adjacentInfo);
                }
            }
        }
    }
}

WangId WangFiller::wangIdFromSurroundings(const TileLayer &back,
                                          const QRegion &region,
                                          QPoint point) const
{
    Cell surroundingCells[8];
    QPoint adjacentPoints[8];
    getSurroundingPoints(point, mStaggeredRenderer, adjacentPoints);

    for (int i = 0; i < 8; ++i) {
        if (!region.contains(adjacentPoints[i]))
            surroundingCells[i] = back.cellAt(adjacentPoints[i]);
    }

    return mWangSet.wangIdFromSurrounding(surroundingCells);
}

bool WangFiller::findBestMatch(const WangSet &wangSet, WangFiller::CellInfo info, WangTile &result) const
{
    const unsigned maskedWangId = info.desired & info.mask;

    RandomPicker<WangTile> matches;
    int lowestPenalty = INT_MAX;

    auto processCandidate = [&] (const WangTile &wangTile) {
        if ((wangTile.wangId() & info.mask) != maskedWangId)
            return;

        int penalty = 0;
        for (int i = 0; i < WangId::NumIndexes; ++i)
            if (wangTile.wangId().indexColor(i) != info.desired.indexColor(i))
                ++penalty;

        // add tile to the candidate list
        if (penalty <= lowestPenalty) {
            if (penalty < lowestPenalty) {
                matches.clear();
                lowestPenalty = penalty;
            }

            matches.add(wangTile, wangSet.wangTileProbability(wangTile));
        }
    };

    // TODO: this is a slow linear search, perhaps we could use a better find algorithm...
    for (const WangTile &wangTile : wangSet.wangTilesByWangId())
        processCandidate(wangTile);

    if (mErasingEnabled)
        processCandidate(WangTile());

    // choose a candidate at random, with consideration for probability
    if (!matches.isEmpty()) {
        result = matches.pick();
        return true;
    }

    return false;
}
