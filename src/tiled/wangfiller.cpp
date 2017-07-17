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

#include "tilelayer.h"
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
                                 bool lookForward) const
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
                adjacentWangId.updateToAdjacent(wangId, (i + 4) % 8);

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

TileLayer *WangFiller::fillRegion(const TileLayer &back,
                            const QRegion &fillRegion,
                            bool lookForward) const
{
    QRect boundingRect = fillRegion.boundingRect();

    TileLayer *tileLayer = new TileLayer(QString(),
                                        boundingRect.x(),
                                        boundingRect.y(),
                                        boundingRect.width(),
                                        boundingRect.height());

    QVector<WangId> wangIds;
    wangIds.resize(tileLayer->width() * tileLayer->height());
    for (int i = 0; i < wangIds.size(); ++i) {
        QPoint point(i % tileLayer->width() + tileLayer->x(),
                     i / tileLayer->width() + tileLayer->y());
        if (fillRegion.contains(point))
            wangIds[i] = wangIdFromSurroundings(back,
                                                fillRegion,
                                                point);
    }

    for (const QRect &rect : fillRegion.rects()) {
        for (int y = rect.top(); y <= rect.bottom(); ++y) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                QPoint currentPoint(x, y);
                int currentIndex = (currentPoint.y() - tileLayer->y()) * tileLayer->width() + (currentPoint.x() - tileLayer->x());
                QList<WangTile> wangTiles = mWangSet->findMatchingWangTiles(wangIds[currentIndex]);

                while(!wangTiles.isEmpty()) {
                    WangTile wangTile = wangTiles.takeAt(qrand() % wangTiles.size());

                    bool fill = true;
                    if (lookForward) {
                        for (int i = 0; i < 8; ++i) {
                            QPoint p = currentPoint + adjacentPoints[i];
                            if (!fillRegion.contains(p) || !tileLayer->cellAt(p - QPoint(tileLayer->x(), tileLayer->y())).isEmpty())
                                continue;
                            p -= QPoint(tileLayer->x(), tileLayer->y());
                            int index = p.y() * tileLayer->width() + p.x();

                            WangId adjacentWangId = wangIds[index];
                            adjacentWangId.updateToAdjacent(wangTile.wangId(), (i + 4) % 8);

                            if (!mWangSet->wildWangIdIsUsed(adjacentWangId)) {
                                if (wangTiles.isEmpty())
                                    fill = true;
                                else
                                    fill = false;

                                break;
                            }
                        }
                    }

                    if (fill) {
                        tileLayer->setCell(currentPoint.x() - tileLayer->x(),
                                          currentPoint.y() - tileLayer->y(),
                                          wangTile.makeCell());

                        for (int i = 0; i < 8; ++i) {
                            QPoint p = currentPoint + adjacentPoints[i];
                            if (!fillRegion.contains(p) || !tileLayer->cellAt(p - QPoint(tileLayer->x(), tileLayer->y())).isEmpty())
                                continue;
                            p -= QPoint(tileLayer->x(), tileLayer->y());
                            int index = p.y() * tileLayer->width() + p.x();
                            wangIds[index].updateToAdjacent(wangTile.wangId(), (i + 4) % 8);
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
                                          QPoint point) const
{
    Cell surroundingCells[8];

    for (int i = 0; i < 8; ++i)
        surroundingCells[i] = getCell(back, front, fillRegion, point + adjacentPoints[i]);

    return mWangSet->wangIdFromSurrounding(surroundingCells);
}

WangId WangFiller::wangIdFromSurroundings(const TileLayer &back,
                                          const QRegion &fillRegion,
                                          QPoint point) const
{
    Cell surroundingCells[8];

    for (int i = 0; i < 8; ++i) {
        QPoint adjacentPoint = point + adjacentPoints[i];
        if (!fillRegion.contains(adjacentPoint) && back.contains(adjacentPoint))
            surroundingCells[i] = back.cellAt(adjacentPoint);
        else
            surroundingCells[i] = Cell();
    }

    return mWangSet->wangIdFromSurrounding(surroundingCells);
}
