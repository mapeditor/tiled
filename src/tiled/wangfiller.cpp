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

static constexpr QPoint aroundTilePoints[WangId::NumIndexes] = {
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
    info.mask.setIndexColor(position, WangId::INDEX_MASK);

    if (!WangId::isCorner(position)) {
        const int cornerA = WangId::nextIndex(position);
        const int cornerB = WangId::previousIndex(position);
        const int adjacentCornerA = WangId::previousIndex(adjacentPosition);
        const int adjacentCornerB = WangId::nextIndex(adjacentPosition);

        info.desired.setIndexColor(cornerA, adjacent.indexColor(adjacentCornerA));
        info.mask.setIndexColor(cornerA, WangId::INDEX_MASK);

        info.desired.setIndexColor(cornerB, adjacent.indexColor(adjacentCornerB));
        info.mask.setIndexColor(cornerB, WangId::INDEX_MASK);
    }
}

void WangFiller::fillRegion(TileLayer &target,
                            const TileLayer &back,
                            const QRegion &region,
                            Grid<CellInfo> grid) const
{
    if (mCorrectionsEnabled) {
        // Determine the desired WangId for all tiles in the region.
#if QT_VERSION < 0x050800
        const auto rects = region.rects();
        for (const QRect &rect : rects) {
#else
        for (const QRect &rect : region) {
#endif
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                for (int x = rect.left(); x <= rect.right(); ++x) {
                    CellInfo info = grid.get(x, y);
                    const auto currentWangId = mWangSet.wangIdOfCell(back.cellAt(x, y));

                    for (int i = 0; i < WangId::NumIndexes; ++i)
                        if (!info.mask.indexColor(i))
                            info.desired.setIndexColor(i, currentWangId.indexColor(i));

                    grid.set(x, y, info);
                }
            }
        }
    } else {
        // Set the Wang IDs at the border of the region to make sure the tiles in
        // the filled region connect with those outside of it.
        auto setDesiredWangId = [&] (int x, int y) {
            const WangId source = wangIdFromSurroundings(back, region, QPoint(x, y));
            CellInfo info = grid.get(x, y);
            for (int i = 0; i < WangId::NumIndexes; ++i) {
                if (!info.mask.indexColor(i)) {
                    if (int color = source.indexColor(i)) {
                        info.desired.setIndexColor(i, color);

                        // When we're not making corrections, require the borders
                        // to match already placed tiles.
                        if (!mCorrectionsEnabled)
                            info.mask.setIndexColor(i, WangId::INDEX_MASK);
                    }
                }
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
    }

    // Determine the bounds of the affected area
    QRect bounds = region.boundingRect();
    int margin = mWangSet.maximumColorDistance() + (mStaggeredRenderer != nullptr);
    bounds.adjust(-margin, -margin, margin, margin);

    // Keep a list of points that need correction
    QVector<QPoint> corrections;

    auto resolve = [&] (int x, int y) {
        const QPoint targetPos(x - target.x(),
                               y - target.y());

        if (target.cellAt(targetPos).checked())
            return;

        Cell cell;
        if (!findBestMatch(target, grid, QPoint(x, y), cell)) {
            // TODO: error feedback
            return;
        }

        cell.setChecked(true);
        target.setCell(targetPos.x(), targetPos.y(), cell);

        const WangId cellWangId = mWangSet.wangIdOfCell(cell);

        // Adjust the desired WangIds for the surrounding tiles based on the placed one
        QPoint adjacentPoints[WangId::NumIndexes];
        getSurroundingPoints(QPoint(x, y), mStaggeredRenderer, adjacentPoints);

        for (int i = 0; i < WangId::NumIndexes; ++i) {
            const QPoint p = adjacentPoints[i];
            if (target.cellAt(p - target.position()).checked())
                continue;

            CellInfo adjacentInfo = grid.get(p);
            updateToAdjacent(adjacentInfo, cellWangId, WangId::oppositeIndex(i));

            // Check if we may need to reconsider a tile outside of our starting region
            if (!WangId::isCorner(i) && mCorrectionsEnabled && bounds.contains(p) && !region.contains(p)) {
                const WangId currentWangId = mWangSet.wangIdOfCell(back.cellAt(p));

                if ((currentWangId & adjacentInfo.mask) != (adjacentInfo.desired & adjacentInfo.mask)) {
                    corrections.append(p);

                    // Synchronize desired WangId with current tile, keeping the masked indexes
                    for (int i = 0; i < WangId::NumIndexes; ++i)
                        if (!adjacentInfo.mask.indexColor(i))
                            adjacentInfo.desired.setIndexColor(i, currentWangId.indexColor(i));
                }
            }

            grid.set(p, adjacentInfo);
        }
    };

    // First process the initial region
#if QT_VERSION < 0x050800
    const auto rects = region.rects();
    for (const QRect &rect : rects) {
#else
    for (const QRect &rect : region) {
#endif
        for (int y = rect.top(); y <= rect.bottom(); ++y)
            for (int x = rect.left(); x <= rect.right(); ++x)
                resolve(x, y);
    }

    // Process each batch of added correction points while avoiding to move
    // around or allocate memory.
    QVector<QPoint> processing;
    while (!corrections.isEmpty()) {
        processing.swap(corrections);
        for (const QPoint &p : processing)
            resolve(p.x(), p.y());
        processing.clear();
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

bool WangFiller::findBestMatch(const TileLayer &target,
                               const Grid<CellInfo> &grid,
                               QPoint position,
                               Cell &result) const
{
    const CellInfo info = grid.get(position);
    const quint64 maskedWangId = info.desired & info.mask;

    RandomPicker<Cell> matches;
    int lowestPenalty = INT_MAX;

    auto processCandidate = [&] (WangId wangId, const Cell &cell) {
        if ((wangId & info.mask) != maskedWangId)
            return;

        int totalPenalty = 0;

        for (int i = 0; i < WangId::NumIndexes; ++i) {
            const int desiredColor = info.desired.indexColor(i);
            const int candidateColor = wangId.indexColor(i);

            if (candidateColor != desiredColor) {
                int penalty = mWangSet.transitionPenalty(desiredColor, candidateColor);

                // If there is no path to the desired color, this isn't a useful transition
                if (penalty < 0) {
                    if (mCorrectionsEnabled) {
                        // When we're doing corrections, we'd rather not choose
                        // this candidate at all because it's impossible to
                        // transition to the desired color.
                        return;
                    } else {
                        penalty = mWangSet.maximumColorDistance() + 1;
                    }
                }

                totalPenalty += penalty;
            }
        }

        // Add tile to the candidate list
        if (totalPenalty <= lowestPenalty) {
            if (totalPenalty < lowestPenalty) {
                matches.clear();
                lowestPenalty = totalPenalty;
            }

            qreal probability = mWangSet.wangIdProbability(wangId);
            if (Tile *tile = cell.tile())
                probability *= tile->probability();

            matches.add(cell, probability);
        }
    };

    const auto &wangIdsAndCells = mWangSet.wangIdsAndCells();
    for (int i = 0, i_end = wangIdsAndCells.size(); i < i_end; ++i)
        processCandidate(wangIdsAndCells[i].wangId, wangIdsAndCells[i].cell);

    if (mErasingEnabled)
        processCandidate(WangId(), Cell());

    // Choose a candidate at random, with consideration for probability
    while (!matches.isEmpty()) {
        result = matches.take();

        // Check if we will be able to place any Wang tile next to this
        // candidate. This can be a relatively expensive check, that we'll only
        // do when we're not making corrections and when the WangSet is not
        // complete.
        if (!mCorrectionsEnabled && !mWangSet.isComplete()) {
            bool discard = false;
            WangId resultWangId = mWangSet.wangIdOfCell(result);

            // Adjust the desired WangIds for the surrounding tiles based on
            // the to be placed one.
            QPoint adjacentPoints[WangId::NumIndexes];
            getSurroundingPoints(position, mStaggeredRenderer, adjacentPoints);

            for (int i = 0; i < WangId::NumIndexes; ++i) {
                const QPoint p = adjacentPoints[i];
                if (target.cellAt(p - target.position()).checked())
                    continue;

                CellInfo adjacentInfo = grid.get(p);
                updateToAdjacent(adjacentInfo, resultWangId, WangId::oppositeIndex(i));

                if (!mWangSet.wangIdIsUsed(adjacentInfo.desired, adjacentInfo.mask)) {
                    discard = true;
                    break;
                }
            }

            if (discard)
                continue;
        }

        return true;
    }

    return false;
}
