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

#include "map.h"
#include "randompicker.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"
#include "wangset.h"

using namespace Tiled;
using namespace Internal;

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

WangFiller::WangFiller(WangSet *wangSet)
    :mWangSet(wangSet)
{
}

void WangFiller::setWangSet(WangSet *wangSet)
{
    mWangSet = wangSet;
}

static QPoint *getSurroundingPoints(QPoint point, StaggeredRenderer *staggeredRenderer, Map::StaggerAxis staggerAxis)
{
    QPoint *adjacentPoints = (QPoint*)malloc(sizeof(QPoint) * 8);
    if (staggeredRenderer) {
        adjacentPoints[0] = staggeredRenderer->topRight(point.x(), point.y());
        adjacentPoints[2] = staggeredRenderer->bottomRight(point.x(), point.y());
        adjacentPoints[4] = staggeredRenderer->bottomLeft(point.x(), point.y());
        adjacentPoints[6] = staggeredRenderer->topLeft(point.x(), point.y());

        if (staggerAxis == Map::StaggerX) {
            adjacentPoints[1] = point + QPoint(2, 0);
            adjacentPoints[3] = point + QPoint(0, 1);
            adjacentPoints[5] = point + QPoint(-2, 0);
            adjacentPoints[7] = point + QPoint(0, -1);
        } else {
            adjacentPoints[1] = point + QPoint(1, 0);
            adjacentPoints[3] = point + QPoint(0, 2);
            adjacentPoints[5] = point + QPoint(-1, 0);
            adjacentPoints[7] = point + QPoint(0, -2);
        }
    } else {
        for (int i = 0; i < 8; ++i)
            adjacentPoints[i] = point + aroundTilePoints[i];
    }

    return adjacentPoints;
}

Cell WangFiller::findFittingCell(const TileLayer &back,
                                 const TileLayer &front,
                                 const QRegion &fillRegion,
                                 QPoint point,
                                 StaggeredRenderer *staggeredRenderer,
                                 Map::StaggerAxis staggerAxis) const
{
    Q_ASSERT(mWangSet);

    QList<WangTile> wangTilesList = mWangSet->findMatchingWangTiles(wangIdFromSurroundings(back,
                                                                                           front,
                                                                                           fillRegion,
                                                                                           point,
                                                                                           staggeredRenderer,
                                                                                           staggerAxis));
    RandomPicker<WangTile> wangTiles;

    for (const WangTile &wangTile : wangTilesList)
        wangTiles.add(wangTile, mWangSet->wangIdProbability(wangTile.wangId()));

    WangTile wangTile;
    if (!mWangSet->isComplete()) {
        //goes through all adjacent, empty tiles and sees if the current wangTile
        //allows them to have at least one fill option.
        while (!wangTiles.isEmpty()) {
            wangTile = wangTiles.take();

            bool continueFlag = false;

            QPoint *adjacentPoints = getSurroundingPoints(point, staggeredRenderer, staggerAxis);

            //now goes through and checks adjacents, continuing if any can't be filled
            for (int i = 0; i < 8; ++i) {
                QPoint adjacentPoint = adjacentPoints[i];

                //check if the point is empty, otherwise, continue.
                if (!getCell(back, front, fillRegion, adjacentPoint).isEmpty())
                    continue;

                WangId adjacentWangId = wangIdFromSurroundings(back,
                                                               front,
                                                               fillRegion,
                                                               adjacentPoint,
                                                               staggeredRenderer,
                                                               staggerAxis);
                WangId wangId = wangTile.wangId();
                adjacentWangId.updateToAdjacent(wangId, (i + 4) % 8);

                if (!mWangSet->wildWangIdIsUsed(adjacentWangId)) {
                    continueFlag = true;
                    break;
                }
            }
            delete adjacentPoints;

            if (!continueFlag)
                break;
        }
    } else if (!wangTiles.isEmpty()) {
        wangTile = wangTiles.pick();
    }

    return wangTile.makeCell();
}

TileLayer *WangFiller::fillRegion(const TileLayer &back,
                                  const QRegion &fillRegion,
                                  StaggeredRenderer *staggeredRenderer,
                                  Map::StaggerAxis staggerAxis) const
{
    Q_ASSERT(mWangSet);

    QRect boundingRect = fillRegion.boundingRect();

    TileLayer *tileLayer = new TileLayer(QString(),
                                         boundingRect.x(),
                                         boundingRect.y(),
                                         boundingRect.width(),
                                         boundingRect.height());

    QVector<WangId> wangIds(tileLayer->width() * tileLayer->height(), 0);
    for (const QRect &rect : fillRegion.rects()) {
        for (int x = rect.left(); x <= rect.right(); ++x) {
            int index = x - tileLayer->x() + (rect.top() - tileLayer->y()) * tileLayer->width();
            wangIds[index] = wangIdFromSurroundings(back,
                                                    fillRegion,
                                                    QPoint(x, rect.top()),
                                                    staggeredRenderer,
                                                    staggerAxis);

            index = x - tileLayer->x() + (rect.bottom() - tileLayer->y())*tileLayer->width();
            wangIds[index] = wangIdFromSurroundings(back,
                                                    fillRegion,
                                                    QPoint(x, rect.bottom()),
                                                    staggeredRenderer,
                                                    staggerAxis);
        }
        for (int y = rect.top() + 1; y < rect.bottom(); ++y) {
            int index = rect.left() - tileLayer->x() + (y - tileLayer->y())*tileLayer->width();
            wangIds[index] = wangIdFromSurroundings(back,
                                                    fillRegion,
                                                    QPoint(rect.left(), y),
                                                    staggeredRenderer,
                                                    staggerAxis);

            index = rect.right() - tileLayer->x() + (y - tileLayer->y())*tileLayer->width();
            wangIds[index] = wangIdFromSurroundings(back,
                                                    fillRegion,
                                                    QPoint(rect.right(), y),
                                                    staggeredRenderer,
                                                    staggerAxis );
        }
    }

    for (const QRect &rect : fillRegion.rects()) {
        for (int y = rect.top(); y <= rect.bottom(); ++y) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                QPoint currentPoint(x, y);
                int currentIndex = (currentPoint.y() - tileLayer->y()) * tileLayer->width() + (currentPoint.x() - tileLayer->x());

                QList<WangTile> wangTilesList = mWangSet->findMatchingWangTiles(wangIds[currentIndex]);
                RandomPicker<WangTile> wangTiles;

                for (const WangTile &wangTile : wangTilesList)
                    wangTiles.add(wangTile, mWangSet->wangIdProbability(wangTile.wangId()));

                while (!wangTiles.isEmpty()) {
                    WangTile wangTile = wangTiles.take();

                    bool fill = true;
                    if (!mWangSet->isComplete()) {

                        QPoint *adjacentPoints = getSurroundingPoints(currentPoint, staggeredRenderer, staggerAxis);

                        for (int i = 0; i < 8; ++i) {
                            QPoint p = adjacentPoints[i];
                            if (!fillRegion.contains(p) || !tileLayer->cellAt(p - tileLayer->position()).isEmpty())
                                continue;
                            p -= tileLayer->position();
                            int index = p.y() * tileLayer->width() + p.x();

                            WangId adjacentWangId = wangIds[index];
                            adjacentWangId.updateToAdjacent(wangTile.wangId(), (i + 4) % 8);

                            if (!mWangSet->wildWangIdIsUsed(adjacentWangId)) {
                                fill = wangTiles.isEmpty();

                                break;
                            }
                        }
                        delete adjacentPoints;
                    }

                    if (fill) {
                        tileLayer->setCell(currentPoint.x() - tileLayer->x(),
                                           currentPoint.y() - tileLayer->y(),
                                           wangTile.makeCell());

                        QPoint *adjacentPoints = getSurroundingPoints(currentPoint, staggeredRenderer, staggerAxis);
                        for (int i = 0; i < 8; ++i) {
                            QPoint p = adjacentPoints[i];
                            if (!fillRegion.contains(p) || !tileLayer->cellAt(p - tileLayer->position()).isEmpty())
                                continue;
                            p -= tileLayer->position();
                            int index = p.y() * tileLayer->width() + p.x();
                            wangIds[index].updateToAdjacent(wangTile.wangId(), (i + 4) % 8);
                        }
                        delete adjacentPoints;

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
    if (!fillRegion.contains(point) && back.contains(point)) {
        return back.cellAt(point);
    } else if (front.contains(point.x() - front.x(), point.y() - front.y())) {
        return front.cellAt(point.x() - front.x(), point.y() - front.y());
    } else {
        static const Cell cell;
        return cell;
    }
}


WangId WangFiller::wangIdFromSurroundings(const TileLayer &back,
                                          const TileLayer &front,
                                          const QRegion &fillRegion,
                                          QPoint point,
                                          StaggeredRenderer *staggeredRenderer,
                                          Map::StaggerAxis staggerAxis) const
{
    Cell surroundingCells[8];

    QPoint *adjacentPoints = getSurroundingPoints(point, staggeredRenderer, staggerAxis);

    for (int i = 0; i < 8; ++i)
        surroundingCells[i] = getCell(back, front, fillRegion, adjacentPoints[i]);
    delete adjacentPoints;

    return mWangSet->wangIdFromSurrounding(surroundingCells);
}

WangId WangFiller::wangIdFromSurroundings(const TileLayer &back,
                                          const QRegion &fillRegion,
                                          QPoint point,
                                          StaggeredRenderer *staggeredRenderer,
                                          Map::StaggerAxis staggerAxis) const
{
    Cell surroundingCells[8];

    QPoint *adjacentPoints = getSurroundingPoints(point, staggeredRenderer, staggerAxis);

    for (int i = 0; i < 8; ++i) {
        if (!fillRegion.contains(adjacentPoints[i]) && back.contains(adjacentPoints[i]))
            surroundingCells[i] = back.cellAt(adjacentPoints[i]);
    }
    free(adjacentPoints);

    return mWangSet->wangIdFromSurrounding(surroundingCells);
}
