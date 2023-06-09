/*
 * geometry.cpp
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include "geometry.h"

#include <QTransform>

namespace Tiled {

/**
 * Returns a lists of points on an ellipse.
 * (x0,y0) is the midpoint
 * (x1,y1) determines the radius.
 *
 * It is adapted from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 * here is the original: http://homepage.smc.edu/kennedy_john/belipse.pdf
 */
QVector<QPoint> pointsOnEllipse(int x0, int y0, int x1, int y1)
{
    QVector<QPoint> ret;
    int x, y;
    int xChange, yChange;
    int ellipseError;
    int twoXSquare, twoYSquare;
    int stoppingX, stoppingY;
    int radiusX = x0 > x1 ? x0 - x1 : x1 - x0;
    int radiusY = y0 > y1 ? y0 - y1 : y1 - y0;

    if (radiusX == 0 && radiusY == 0)
        return ret;

    twoXSquare = 2 * radiusX * radiusX;
    twoYSquare = 2 * radiusY * radiusY;
    x = radiusX;
    y = 0;
    xChange = radiusY * radiusY * (1 - 2 * radiusX);
    yChange = radiusX * radiusX;
    ellipseError = 0;
    stoppingX = twoYSquare*radiusX;
    stoppingY = 0;
    while (stoppingX >= stoppingY) {
        ret += QPoint(x0 + x, y0 + y);
        ret += QPoint(x0 - x, y0 + y);
        ret += QPoint(x0 + x, y0 - y);
        ret += QPoint(x0 - x, y0 - y);
        y++;
        stoppingY += twoXSquare;
        ellipseError += yChange;
        yChange += twoXSquare;
        if ((2 * ellipseError + xChange) > 0) {
            x--;
            stoppingX -= twoYSquare;
            ellipseError += xChange;
            xChange += twoYSquare;
        }
    }
    x = 0;
    y = radiusY;
    xChange = radiusY * radiusY;
    yChange = radiusX * radiusX * (1 - 2 * radiusY);
    ellipseError = 0;
    stoppingX = 0;
    stoppingY = twoXSquare * radiusY;
    while (stoppingX <= stoppingY) {
        ret += QPoint(x0 + x, y0 + y);
        ret += QPoint(x0 - x, y0 + y);
        ret += QPoint(x0 + x, y0 - y);
        ret += QPoint(x0 - x, y0 - y);
        x++;
        stoppingX += twoYSquare;
        ellipseError += xChange;
        xChange += twoYSquare;
        if ((2 * ellipseError + yChange) > 0) {
            y--;
            stoppingY -= twoXSquare;
            ellipseError += yChange;
            yChange += twoXSquare;
        }
    }

    return ret;
}

/**
 * returns an elliptical region centered at x0,y0 with radius determined by x1,y1
 */
QRegion ellipseRegion(int x0, int y0, int x1, int y1)
{
    QRegion ret;
    int x, y;
    int xChange, yChange;
    int ellipseError;
    int twoXSquare, twoYSquare;
    int stoppingX, stoppingY;
    int radiusX = x0 > x1 ? x0 - x1 : x1 - x0;
    int radiusY = y0 > y1 ? y0 - y1 : y1 - y0;

    if (radiusX == 0 && radiusY == 0)
        return ret;

    twoXSquare = 2 * radiusX * radiusX;
    twoYSquare = 2 * radiusY * radiusY;
    x = radiusX;
    y = 0;
    xChange = radiusY * radiusY * (1 - 2 * radiusX);
    yChange = radiusX * radiusX;
    ellipseError = 0;
    stoppingX = twoYSquare*radiusX;
    stoppingY = 0;
    while (stoppingX >= stoppingY) {
        ret += QRect(-x, y, x * 2, 1);
        ret += QRect(-x, -y, x * 2, 1);
        y++;
        stoppingY += twoXSquare;
        ellipseError += yChange;
        yChange += twoXSquare;
        if ((2 * ellipseError + xChange) > 0) {
            x--;
            stoppingX -= twoYSquare;
            ellipseError += xChange;
            xChange += twoYSquare;
        }
    }
    x = 0;
    y = radiusY;
    xChange = radiusY * radiusY;
    yChange = radiusX * radiusX * (1 - 2 * radiusY);
    ellipseError = 0;
    stoppingX = 0;
    stoppingY = twoXSquare * radiusY;
    while (stoppingX <= stoppingY) {
        ret += QRect(-x, y, x * 2, 1);
        ret += QRect(-x, -y, x * 2, 1);
        x++;
        stoppingX += twoYSquare;
        ellipseError += xChange;
        xChange += twoYSquare;
        if ((2 * ellipseError + yChange) > 0) {
            y--;
            stoppingY -= twoXSquare;
            ellipseError += yChange;
            yChange += twoXSquare;
        }
    }

    return ret.translated(x0, y0);
}

/**
 * Returns the lists of points on a line from (x0,y0) to (x1,y1).
 *
 * This is an implementation of Bresenham's line algorithm, initially copied
 * from http://en.wikipedia.org/wiki/Bresenham's_line_algorithm#Optimization
 * changed to C++ syntax.
 *
 * When the \a manhattan option (named after "Manhattan distance") is set to
 * true, the points on the line can't take diagonal steps.
 */
QVector<QPoint> pointsOnLine(int x0, int y0, int x1, int y1, bool manhattan)
{
    QVector<QPoint> ret;

    const bool steep = qAbs(y1 - y0) > qAbs(x1 - x0);
    if (steep) {
        qSwap(x0, y0);
        qSwap(x1, y1);
    }

    const bool reverse = x0 > x1;
    if (reverse) {
        qSwap(x0, x1);
        qSwap(y0, y1);
    }

    const int deltax = x1 - x0;
    const int deltay = qAbs(y1 - y0);
    int error = deltax / 2;
    int ystep;
    int y = y0;

    if (y0 < y1)
        ystep = 1;
    else
        ystep = -1;

    ret.reserve(deltax + 1 + (manhattan ? deltay : 0));

    for (int x = x0; x <= x1; x++) {
        ret += steep ? QPoint(y, x) : QPoint(x, y);
        error = error - deltay;
        if (error < 0) {
             y = y + ystep;
             error = error + deltax;

             if (manhattan && x < x1)
                ret += steep ? QPoint(y, x) : QPoint(x, y);
        }
    }

    if (reverse)
        std::reverse(ret.begin(), ret.end());

    return ret;
}

/**
 * Checks if a given rectangle \a rect is coherent to another given \a region.
 * 'coherent' means that either the rectangle is overlapping the region or
 * the rectangle contains at least one tile, which is a neighbour to a tile,
 * which belongs to the region.
 */
static bool isCoherentTo(const QRect &rect, const QRegion &region)
{
    return region.intersects(rect.adjusted(-1, -1, 1, 1));
}

/**
 * Calculates all coherent regions occupied by the given \a region.
 * Returns an array of regions, where each region is coherent in itself.
 */
QVector<QRegion> coherentRegions(const QRegion &region)
{
    QVector<QRegion> result;
    QVector<QRect> rects;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    rects.reserve(static_cast<int>(region.end() - region.begin()));
    for (const QRect &rect : region)
        rects.append(rect);
#else
    rects = QVector<QRect>(region.begin(), region.end());
#endif

    while (!rects.isEmpty()) {
        QRegion newCoherentRegion = rects.takeLast();

        // Add up all coherent rects until there is no rect left which is
        // coherent to the newly created region.
        bool foundRect = true;
        while (foundRect) {
            foundRect = false;
            for (int i = rects.size() - 1; i >= 0; --i) {
                if (isCoherentTo(rects.at(i), newCoherentRegion)) {
                    newCoherentRegion += rects.at(i);
                    rects.remove(i);
                    foundRect = true;
                }
            }
        }
        result += newCoherentRegion;
    }
    return result;
}

/**
 * Returns a transform that rotates by \a rotation degrees around the given
 * \a position.
 */
QTransform rotateAt(const QPointF &position, qreal rotation)
{
    QTransform transform;
    transform.translate(position.x(), position.y());
    transform.rotate(rotation);
    transform.translate(-position.x(), -position.y());
    return transform;
}

} // namespace Tiled
