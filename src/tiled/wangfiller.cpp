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
                       const StaggeredRenderer *staggeredRenderer,
                       Map::StaggerAxis staggerAxis)
    : mWangSet(wangSet)
    , mStaggeredRenderer(staggeredRenderer)
    , mStaggerAxis(staggerAxis)
{
}

static void getSurroundingPoints(QPoint point,
                                 const StaggeredRenderer *staggeredRenderer,
                                 Map::StaggerAxis staggerAxis,
                                 QPoint *points)
{
    if (staggeredRenderer) {
        points[0] = staggeredRenderer->topRight(point.x(), point.y());
        points[2] = staggeredRenderer->bottomRight(point.x(), point.y());
        points[4] = staggeredRenderer->bottomLeft(point.x(), point.y());
        points[6] = staggeredRenderer->topLeft(point.x(), point.y());

        if (staggerAxis == Map::StaggerX) {
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

Cell WangFiller::findFittingCell(const TileLayer &back,
                                 const TileLayer &front,
                                 const QRegion &fillRegion,
                                 QPoint point) const
{
    const WangId wangId = wangIdFromSurroundings(back, front, fillRegion, point);
    const auto wangTilesList = mWangSet.findMatchingWangTiles(wangId);

    RandomPicker<WangTile> wangTiles;

    for (const WangTile &wangTile : wangTilesList)
        wangTiles.add(wangTile, mWangSet.wangTileProbability(wangTile));

    if (wangTiles.isEmpty())
        return {};

    // If the Wang set is complete (has all possible combinations), we can pick
    // any of the fitting Wang tiles.
    if (mWangSet.isComplete())
        return wangTiles.pick().makeCell();

    WangTile wangTile;

    // If the set is not complete, we're going to be more careful, trying to
    // pick only Wang tiles for which we can find fitting neighbors on all
    // sides (a rather expensive check, though without it we'll often get
    // situations where we can't find a fitting Wang tile).
    while (!wangTiles.isEmpty()) {
        wangTile = wangTiles.take();

        bool continueFlag = false;

        QPoint adjacentPoints[8];
        getSurroundingPoints(point, mStaggeredRenderer, mStaggerAxis, adjacentPoints);

        // now goes through and checks adjacents, continuing if any can't be filled
        for (int i = 0; i < 8; ++i) {
            QPoint adjacentPoint = adjacentPoints[i];

            // check if the point is empty, otherwise, continue.
            if (!getCell(back, front, fillRegion, adjacentPoint).isEmpty())
                continue;

            WangId adjacentWangId = wangIdFromSurroundings(back,
                                                           front,
                                                           fillRegion,
                                                           adjacentPoint);
            WangId wangId = wangTile.wangId();
            adjacentWangId.updateToAdjacent(wangId, WangId::oppositeIndex(i));

            if (!mWangSet.wildWangIdIsUsed(adjacentWangId)) {
                continueFlag = true;
                break;
            }
        }

        if (!continueFlag)
            break;
    }

    return wangTile.makeCell();
}

std::unique_ptr<TileLayer> WangFiller::fillRegion(const TileLayer &back,
                                                  const QRegion &fillRegion) const
{
    const QRect boundingRect = fillRegion.boundingRect();

    std::unique_ptr<TileLayer> tileLayer { new TileLayer(QString(),
                                                         boundingRect.x(),
                                                         boundingRect.y(),
                                                         boundingRect.width(),
                                                         boundingRect.height()) };

    QVector<WangId> wangIds(tileLayer->width() * tileLayer->height(), 0);
#if QT_VERSION < 0x050800
    const auto rects = fillRegion.rects();
    for (const QRect &rect : rects) {
#else
    for (const QRect &rect : fillRegion) {
#endif
        for (int x = rect.left(); x <= rect.right(); ++x) {
            int index = x - tileLayer->x() + (rect.top() - tileLayer->y()) * tileLayer->width();
            wangIds[index] = wangIdFromSurroundings(back,
                                                    fillRegion,
                                                    QPoint(x, rect.top()));

            index = x - tileLayer->x() + (rect.bottom() - tileLayer->y()) * tileLayer->width();
            wangIds[index] = wangIdFromSurroundings(back,
                                                    fillRegion,
                                                    QPoint(x, rect.bottom()));
        }
        for (int y = rect.top() + 1; y < rect.bottom(); ++y) {
            int index = rect.left() - tileLayer->x() + (y - tileLayer->y()) * tileLayer->width();
            wangIds[index] = wangIdFromSurroundings(back,
                                                    fillRegion,
                                                    QPoint(rect.left(), y));

            index = rect.right() - tileLayer->x() + (y - tileLayer->y()) * tileLayer->width();
            wangIds[index] = wangIdFromSurroundings(back,
                                                    fillRegion,
                                                    QPoint(rect.right(), y));
        }
    }

#if QT_VERSION < 0x050800
    for (const QRect &rect : rects) {
#else
    for (const QRect &rect : fillRegion) {
#endif
        for (int y = rect.top(); y <= rect.bottom(); ++y) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                const QPoint currentPoint(x, y);
                const int currentIndex = (currentPoint.y() - tileLayer->y()) * tileLayer->width() + (currentPoint.x() - tileLayer->x());

                const auto wangTilesList = mWangSet.findMatchingWangTiles(wangIds[currentIndex]);
                RandomPicker<WangTile> wangTiles;

                for (const WangTile &wangTile : wangTilesList)
                    wangTiles.add(wangTile, mWangSet.wangTileProbability(wangTile));

                while (!wangTiles.isEmpty()) {
                    const WangTile wangTile = wangTiles.take();

                    bool fill = true;
                    if (!mWangSet.isComplete()) {
                        QPoint adjacentPoints[8];
                        getSurroundingPoints(currentPoint, mStaggeredRenderer, mStaggerAxis, adjacentPoints);

                        for (int i = 0; i < 8; ++i) {
                            QPoint p = adjacentPoints[i];
                            if (!fillRegion.contains(p) || !tileLayer->cellAt(p - tileLayer->position()).isEmpty())
                                continue;
                            p -= tileLayer->position();
                            int index = p.y() * tileLayer->width() + p.x();

                            WangId adjacentWangId = wangIds[index];
                            adjacentWangId.updateToAdjacent(wangTile.wangId(), WangId::oppositeIndex(i));

                            if (!mWangSet.wildWangIdIsUsed(adjacentWangId)) {
                                fill = wangTiles.isEmpty();

                                break;
                            }
                        }
                    }

                    if (fill) {
                        tileLayer->setCell(currentPoint.x() - tileLayer->x(),
                                           currentPoint.y() - tileLayer->y(),
                                           wangTile.makeCell());
                        QPoint adjacentPoints[8];
                        getSurroundingPoints(currentPoint, mStaggeredRenderer, mStaggerAxis, adjacentPoints);
                        for (int i = 0; i < 8; ++i) {
                            QPoint p = adjacentPoints[i];
                            if (!fillRegion.contains(p) || !tileLayer->cellAt(p - tileLayer->position()).isEmpty())
                                continue;
                            p -= tileLayer->position();
                            int index = p.y() * tileLayer->width() + p.x();
                            wangIds[index].updateToAdjacent(wangTile.wangId(), WangId::oppositeIndex(i));
                        }
                        break;
                    }
                }
            }
        }
    }

    return tileLayer;
}

const Cell &WangFiller::getCell(const TileLayer &back,
                                const TileLayer &front,
                                const QRegion &fillRegion,
                                QPoint point) const
{
    if (!fillRegion.contains(point))
        return back.cellAt(point);
    else
        return front.cellAt(point.x() - front.x(), point.y() - front.y());
}


WangId WangFiller::wangIdFromSurroundings(const TileLayer &back,
                                          const TileLayer &front,
                                          const QRegion &fillRegion,
                                          QPoint point) const
{
    Cell surroundingCells[8];
    QPoint adjacentPoints[8];
    getSurroundingPoints(point, mStaggeredRenderer, mStaggerAxis, adjacentPoints);

    for (int i = 0; i < 8; ++i)
        surroundingCells[i] = getCell(back, front, fillRegion, adjacentPoints[i]);

    return mWangSet.wangIdFromSurrounding(surroundingCells);
}

WangId WangFiller::wangIdFromSurroundings(const TileLayer &back,
                                          const QRegion &fillRegion,
                                          QPoint point) const
{
    Cell surroundingCells[8];
    QPoint adjacentPoints[8];
    getSurroundingPoints(point, mStaggeredRenderer, mStaggerAxis, adjacentPoints);

    for (int i = 0; i < 8; ++i) {
        if (!fillRegion.contains(adjacentPoints[i]))
            surroundingCells[i] = back.cellAt(adjacentPoints[i]);
    }

    return mWangSet.wangIdFromSurrounding(surroundingCells);
}
