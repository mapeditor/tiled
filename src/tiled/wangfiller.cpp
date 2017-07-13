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
#include "wangset.h"

using namespace Tiled;
using namespace Internal;

static const QPoint adjacentPoints[] = {
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

Cell WangFiller::findFittingCell(const TileLayer &back,
                                 const TileLayer &front,
                                 const QRegion &fillRegion,
                                 QPoint point,
                                 bool lookForward)
{
    Q_ASSERT(mWangSet);

    QList<WangTile> wangTiles = mWangSet->findMatchingWangTiles(wangIdFromSurroundings(back, front, fillRegion, point));

    WangTile wangTile;
    if (lookForward) {
        //goes through all adjacent, empty tiles and sees if the current wangTile
        //allows them to have at least one fill option.
        while (!wangTiles.isEmpty()) {
            wangTile = wangTiles.takeAt(qrand() % wangTiles.size());

            bool continueFlag = false;
            //now goes through and checks adjacents, continuing if any can't be filled
            for (int i = 0; i < 8; ++i) {
                QPoint adjacentPoint = point + adjacentPoints[i];

                //check if the point is empty, otherwise, continue.
                if (!getCell(back, front, fillRegion, adjacentPoint).isEmpty())
                    continue;

                WangId adjacentWangId = wangIdFromSurroundings(back,
                                                               front,
                                                               fillRegion,
                                                               adjacentPoint);

                WangId wangId = wangTile.wangId();
                switch(adjacentPoints[i].x()) {
                case -1: {
                    switch(adjacentPoints[i].y()) {
                    case -1:
                        if (mWangSet->cornerColors() > 1)
                            adjacentWangId.setCornerColor(1, wangId.cornerColor(3));
                        break;
                    case 0:
                        if (mWangSet->cornerColors() > 1) {
                            adjacentWangId.setCornerColor(0, wangId.cornerColor(3));
                            adjacentWangId.setCornerColor(1, wangId.cornerColor(2));
                        }
                        if (mWangSet->edgeColors() > 1)
                            adjacentWangId.setEdgeColor(1, wangId.edgeColor(3));
                        break;
                    case 1:
                        if (mWangSet->cornerColors() > 1)
                            adjacentWangId.setCornerColor(0, wangId.cornerColor(2));
                        break;
                    }
                }
                    break;
                case 0: {
                    switch(adjacentPoints[i].y()) {
                    case -1:
                        if (mWangSet->cornerColors() > 1) {
                            adjacentWangId.setCornerColor(2, wangId.cornerColor(3));
                            adjacentWangId.setCornerColor(1, wangId.cornerColor(0));
                        }
                        if (mWangSet->edgeColors() > 1)
                            adjacentWangId.setEdgeColor(2, wangId.edgeColor(0));
                        break;
                    case 1:
                        if (mWangSet->cornerColors() > 1) {
                            adjacentWangId.setCornerColor(3, wangId.cornerColor(2));
                            adjacentWangId.setCornerColor(0, wangId.cornerColor(1));
                        }
                        if (mWangSet->edgeColors() > 1)
                            adjacentWangId.setEdgeColor(0, wangId.edgeColor(2));
                        break;
                    }
                }
                    break;
                case 1: {
                    switch(adjacentPoints[i].y()) {
                    case -1:
                        if (mWangSet->cornerColors() > 1)
                            adjacentWangId.setCornerColor(2, wangId.cornerColor(0));
                        break;
                    case 0:
                        if (mWangSet->cornerColors() > 1) {
                            adjacentWangId.setCornerColor(3, wangId.cornerColor(0));
                            adjacentWangId.setCornerColor(2, wangId.cornerColor(1));
                        }
                        if (mWangSet->edgeColors() > 1)
                            adjacentWangId.setEdgeColor(3, wangId.edgeColor(1));
                        break;
                    case 1:
                        if (mWangSet->cornerColors() > 1)
                            adjacentWangId.setCornerColor(3, wangId.cornerColor(1));
                        break;
                    }
                }
                    break;
                }

                if (!mWangSet->wildWangIdIsUsed(adjacentWangId)) {
                    continueFlag = true;
                    break;
                }
            }

            if (!continueFlag)
                break;
        }
    } else if (!wangTiles.isEmpty()) {
        wangTile = wangTiles.at(qrand() % wangTiles.size());
    }

    return wangTile.makeCell();
}

const Cell &WangFiller::getCell(const TileLayer &back,
                                const TileLayer &front,
                                const QRegion &fillRegion,
                                QPoint point) const
{
    if (!fillRegion.contains(QPoint(point.x() + front.x(), point.y() + front.y()))
            && back.contains(point.x() + front.x(), point.y() + front.y())) {
        return back.cellAt(point.x() + front.x(), point.y() + front.y());
    } else if (front.contains(point.x(), point.y())) {
        return front.cellAt(point.x(), point.y());
    } else {
        static const Cell cell;
        return cell;
    }
}

WangId WangFiller::wangIdFromSurroundings(const TileLayer &back,
                                          const TileLayer &front,
                                          const QRegion &fillRegion,
                                          QPoint point) const
{
    Cell surroundingCells[] = {
        getCell(back, front, fillRegion, point + QPoint( 0, -1)),
        getCell(back, front, fillRegion, point + QPoint( 1, -1)),
        getCell(back, front, fillRegion, point + QPoint( 1,  0)),
        getCell(back, front, fillRegion, point + QPoint( 1,  1)),
        getCell(back, front, fillRegion, point + QPoint( 0,  1)),
        getCell(back, front, fillRegion, point + QPoint(-1,  1)),
        getCell(back, front, fillRegion, point + QPoint(-1,  0)),
        getCell(back, front, fillRegion, point + QPoint(-1, -1))
    };

    return mWangSet->wangIdFromSurrounding(surroundingCells);
}
