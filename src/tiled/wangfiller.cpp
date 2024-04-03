/*
 * wangfiller.cpp
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

#include "wangfiller.h"

#include "grid.h"
#include "hexagonalrenderer.h"
#include "map.h"
#include "randompicker.h"
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
                       const TileLayer &back,
                       const MapRenderer *mapRenderer)
    : mWangSet(wangSet)
    , mBack(back)
    , mMapRenderer(mapRenderer)
    , mHexagonalRenderer(dynamic_cast<const HexagonalRenderer*>(mapRenderer))
{
}

void WangFiller::setRegion(const QRegion &region)
{
    mFillRegion.region = region;
}

WangFiller::CellInfo &WangFiller::changePosition(QPoint pos)
{
    CellInfo &info = mFillRegion.grid.add(pos);

    // Initialize the desired WangId when necessary, and make sure the location
    // is part of the to be processed region.
    if (info == CellInfo()) {
        info.desired = mWangSet.wangIdOfCell(mBack.cellAt(pos));
        mFillRegion.region += QRect(pos, pos);
    }

    return info;
}

void WangFiller::setWangIndex(QPoint pos, WangId::Index index, int color)
{
    CellInfo &info = changePosition(pos);

    // Set the requested color at the given index
    info.desired.setIndexColor(index, color);
    info.mask.setIndexColor(index, WangId::INDEX_MASK);
}

void WangFiller::setCorner(QPoint vertexPos, int color)
{
    if (mHexagonalRenderer) {
        const QPoint topLeft = mHexagonalRenderer->topLeft(vertexPos.x(), vertexPos.y());

        setWangIndex(mHexagonalRenderer->topRight(vertexPos.x(), vertexPos.y()), WangId::BottomLeft, color);
        setWangIndex(vertexPos, WangId::TopLeft, color);
        setWangIndex(topLeft, WangId::TopRight, color);
        setWangIndex(mHexagonalRenderer->topRight(topLeft.x(), topLeft.y()), WangId::BottomRight, color);
    } else {
        setWangIndex(vertexPos + QPoint( 0, -1), WangId::BottomLeft, color);
        setWangIndex(vertexPos + QPoint( 0,  0), WangId::TopLeft, color);
        setWangIndex(vertexPos + QPoint(-1,  0), WangId::TopRight, color);
        setWangIndex(vertexPos + QPoint(-1, -1), WangId::BottomRight, color);
    }
}

void WangFiller::setEdge(QPoint pos, WangId::Index index, int color)
{
    setWangIndex(pos, index, color);

    const auto oppositeIndex = WangId::oppositeIndex(index);
    QPoint dirPoint;

    if (mHexagonalRenderer) {
        switch (index) {
        case WangId::Top:
            dirPoint = mHexagonalRenderer->topRight(pos.x(), pos.y());
            break;
        case WangId::Right:
            dirPoint = mHexagonalRenderer->bottomRight(pos.x(), pos.y());
            break;
        case WangId::Bottom:
            dirPoint = mHexagonalRenderer->bottomLeft(pos.x(), pos.y());
            break;
        case WangId::Left:
            dirPoint = mHexagonalRenderer->topLeft(pos.x(), pos.y());
            break;
        default:    // Other color indexes not handled when painting edges
            return;
        }
    } else {
        dirPoint = pos + aroundTilePoints[index];
    }

    setWangIndex(dirPoint, oppositeIndex, color);
}

static void getSurroundingPoints(QPoint point,
                                 const HexagonalRenderer *hexagonalRenderer,
                                 QPoint *points)
{
    if (hexagonalRenderer) {
        points[0] = hexagonalRenderer->topRight(point.x(), point.y());
        points[2] = hexagonalRenderer->bottomRight(point.x(), point.y());
        points[4] = hexagonalRenderer->bottomLeft(point.x(), point.y());
        points[6] = hexagonalRenderer->topLeft(point.x(), point.y());

        if (hexagonalRenderer->map()->staggerAxis() == Map::StaggerX) {
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
 * Matches the given \a adjacentInfo's edges/corners with the given \a wangId.
 * The position of the adjacent info is given by \a adjacentPosition. Also sets
 * the mask for the given corner / side.
 */
static void updateAdjacent(WangFiller::CellInfo &adjacentInfo, WangId wangId, int adjacentIndex)
{
    const int index = WangId::oppositeIndex(adjacentIndex);

    adjacentInfo.desired.setIndexColor(index, wangId.indexColor(adjacentIndex));
    adjacentInfo.mask.setIndexColor(index, WangId::INDEX_MASK);

    if (!WangId::isCorner(index)) {
        const int cornerA = WangId::nextIndex(index);
        const int cornerB = WangId::previousIndex(index);
        const int adjacentCornerA = WangId::previousIndex(adjacentIndex);
        const int adjacentCornerB = WangId::nextIndex(adjacentIndex);

        adjacentInfo.desired.setIndexColor(cornerA, wangId.indexColor(adjacentCornerA));
        adjacentInfo.mask.setIndexColor(cornerA, WangId::INDEX_MASK);

        adjacentInfo.desired.setIndexColor(cornerB, wangId.indexColor(adjacentCornerB));
        adjacentInfo.mask.setIndexColor(cornerB, WangId::INDEX_MASK);
    }
}

/**
 * Applies the scheduled Wang changes to the \a target layer.
 */
void WangFiller::apply(TileLayer &target)
{
    mInvalidRegion = QRegion();

    auto &grid = mFillRegion.grid;
    auto &region = mFillRegion.region;

    // Don't try to make changes outside of a fixed map. Instead, to still
    // provide some feedback in previews, explicitly mark this area as empty.
    if (!mMapRenderer->map()->infinite()) {
        const auto emptyRegion = region.subtracted(mBack.rect());

        Cell empty;
        empty.setChecked(true);

        for (const QRect &rect : emptyRegion)
            for (int y = rect.top(); y <= rect.bottom(); ++y)
                for (int x = rect.left(); x <= rect.right(); ++x)
                    target.setCell(x - target.x(), y - target.y(), empty);

        region &= mBack.rect();
    }

    if (!mCorrectionsEnabled) {
        // Set the Wang IDs at the border of the region to prefer the tiles in
        // the filled region to connect with those outside of it.
        auto setDesiredWangId = [&] (int x, int y, WangId mask) {
            const WangId surroundings = wangIdFromSurroundings(QPoint(x, y));
            CellInfo &info = grid.add(x, y);

            // Don't override explicitly set indexes and don't override with
            // unset indices from surroundings.
            mask &= ~(info.mask | surroundings.mask(WangId::INDEX_MASK));
            info.desired.mergeWith(surroundings, mask);
        };

        for (const QRect &rect : region) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                setDesiredWangId(x, rect.top(), WangId::MaskTopSide);
                setDesiredWangId(x, rect.bottom(), WangId::MaskBottomSide);
            }
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                setDesiredWangId(rect.left(), y, WangId::MaskLeftSide);
                setDesiredWangId(rect.right(), y, WangId::MaskRightSide);
            }
        }
    }

    // Determine the bounds of the affected area
    QRect bounds = region.boundingRect();
    int margin = mWangSet.maximumColorDistance() + (mHexagonalRenderer != nullptr);
    bounds.adjust(-margin, -margin, margin, margin);

    // Don't try to make corrections outside of a fixed map
    if (!mMapRenderer->map()->infinite())
        bounds &= mBack.rect();

    // Keep a list of points that need correction
    QVector<QPoint> corrections;

    auto resolve = [&] (int x, int y) {
        const QPoint targetPos(x - target.x(),
                               y - target.y());

        if (target.cellAt(targetPos).checked())
            return;

        Cell cell;
        if (!findBestMatch(target, grid, QPoint(x, y), cell)) {
            mInvalidRegion += QRect(x, y, 1, 1);
            return;
        }

        cell.setChecked(true);
        target.setCell(targetPos.x(), targetPos.y(), cell);

        const WangId cellWangId = mWangSet.wangIdOfCell(cell);

        // Adjust the desired WangIds for the surrounding tiles based on the placed one
        QPoint adjacentPoints[WangId::NumIndexes];
        getSurroundingPoints(QPoint(x, y), mHexagonalRenderer, adjacentPoints);

        for (int i = 0; i < WangId::NumIndexes; ++i) {
            const QPoint p = adjacentPoints[i];
            if (target.cellAt(p - target.position()).checked())
                continue;

            CellInfo &adjacentInfo = grid.add(p);
            updateAdjacent(adjacentInfo, cellWangId, i);

            // Check if we may need to reconsider a tile outside of our starting region
            if (!WangId::isCorner(i) && mCorrectionsEnabled && bounds.contains(p) && !region.contains(p)) {
                const WangId currentWangId = mWangSet.wangIdOfCell(mBack.cellAt(p));

                if ((currentWangId & adjacentInfo.mask) != (adjacentInfo.desired & adjacentInfo.mask)) {
                    corrections.append(p);

                    // Synchronize desired WangId with current tile, keeping the masked indexes
                    for (int i = 0; i < WangId::NumIndexes; ++i)
                        if (!adjacentInfo.mask.indexColor(i))
                            adjacentInfo.desired.setIndexColor(i, currentWangId.indexColor(i));
                }
            }
        }
    };

    // First process the initial region
    for (const QRect &rect : region) {
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

    mFillRegion = FillRegion();
}

/**
 * Returns a WangId matching that of the provided \a surroundingWangIds.
 *
 * This is based off a provided array, { 0, 1, 2, 3, 4, 5, 6, 7 },
 * which corresponds to:
 *
 *      7|0|1
 *      6|X|2
 *      5|4|3
 */
static WangId wangIdFromSurrounding(const WangId surroundingWangIds[])
{
    WangId id = WangId::FULL_MASK;

    for (int i : { WangId::Top, WangId::Right, WangId::Bottom, WangId::Left })
        id.setIndexColor(i, surroundingWangIds[i].indexColor(WangId::oppositeIndex(i)));

    for (int i : { WangId::TopRight, WangId::BottomRight, WangId::BottomLeft, WangId::TopLeft }) {
        int color = surroundingWangIds[i].indexColor(WangId::oppositeIndex(i));

        // Each corner has two additional connecting tiles on the side, from which a color could be derived.
        if (color == WangId::INDEX_MASK || !color) {
            int leftSideCorner = surroundingWangIds[WangId::previousIndex(i)].indexColor((i + 2) % WangId::NumIndexes);
            if (leftSideCorner != WangId::INDEX_MASK)
                color = leftSideCorner;
        }

        if (color == WangId::INDEX_MASK || !color) {
            int rightSideCorner = surroundingWangIds[WangId::nextIndex(i)].indexColor((i + 6) % WangId::NumIndexes);
            if (rightSideCorner != WangId::INDEX_MASK)
                color = rightSideCorner;
        }

        id.setIndexColor(i, color);
    }

    return id;
}

WangId WangFiller::wangIdFromSurroundings(QPoint point) const
{
    WangId wangIds[WangId::NumIndexes];
    QPoint adjacentPoints[8];
    getSurroundingPoints(point, mHexagonalRenderer, adjacentPoints);

    for (int i = 0; i < WangId::NumIndexes; ++i) {
        wangIds[i] = WangId::FULL_MASK;

        const auto &cell = mBack.cellAt(adjacentPoints[i]);
        if (cell.isEmpty())
            continue;

        if (mFillRegion.region.contains(adjacentPoints[i]))
            continue;

        wangIds[i] = mWangSet.wangIdOfCell(cell);
    }

    return wangIdFromSurrounding(wangIds);
}

bool WangFiller::findBestMatch(const TileLayer &target,
                               const Grid<CellInfo> &grid,
                               QPoint position,
                               Cell &result) const
{
    const CellInfo info = grid.get(position);
    const WangId maskedWangId = info.desired & info.mask;

    RandomPicker<Cell> matches;
    int lowestPenalty = INT_MAX;

    auto processCandidate = [&] (WangId wangId, const Cell &cell) {
        if ((wangId & info.mask) != maskedWangId)
            return;

        int totalPenalty = 0;

        for (int i = 0; i < WangId::NumIndexes; ++i) {
            const int desiredColor = info.desired.indexColor(i);
            if (desiredColor == WangId::INDEX_MASK)
                continue;

            const int candidateColor = wangId.indexColor(i);
            if (desiredColor == candidateColor)
                continue;

            int penalty = mWangSet.transitionPenalty(desiredColor, candidateColor);

            // If there is no path to the desired color, this isn't a useful transition
            if (penalty < 0) {
                if (mCorrectionsEnabled) {
                    // When we're doing corrections, we'd rather not choose
                    // this candidate at all because it's impossible to
                    // transition to the desired color.
                    return;
                }

                penalty = mWangSet.maximumColorDistance() + 1;
            }

            totalPenalty += penalty;
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
    for (const auto &wangIdAndCell : wangIdsAndCells)
        processCandidate(wangIdAndCell.wangId, wangIdAndCell.cell);

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
            const WangId resultWangId = mWangSet.wangIdOfCell(result);
            bool discard = false;

            // Adjust the desired WangIds for the surrounding tiles based on
            // the to be placed one.
            QPoint adjacentPoints[WangId::NumIndexes];
            getSurroundingPoints(position, mHexagonalRenderer, adjacentPoints);

            for (int i = 0; i < WangId::NumIndexes; ++i) {
                const QPoint p = adjacentPoints[i];
                if (target.cellAt(p - target.position()).checked())
                    continue;

                CellInfo adjacentInfo = grid.get(p);
                updateAdjacent(adjacentInfo, resultWangId, i);

                if (adjacentInfo.desired && !mWangSet.wangIdIsUsed(adjacentInfo.desired, adjacentInfo.mask)) {
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
