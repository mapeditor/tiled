/*
 * geometry.cpp
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 * Copyright 2020, Zingl Alois
 * Copyright 2017-2023, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
 * (xm,ym) is the midpoint
 * (a,b) determines the radii.
 *
 * From "Bresenham Curve Rasterizing Algorithms".
 *
 * @version V20.15 april 2020
 * @copyright MIT open-source license software
 * @url https://github.com/zingl/Bresenham
 * @author  Zingl Alois
 */
QVector<QPoint> pointsOnEllipse(int xm, int ym, int a, int b)
{
    QVector<QPoint> ret;

    long x = -a, y = 0;          /* II. quadrant from bottom left to top right */
    long e2 = b, dx = (1+2*x)*e2*e2;                       /* error increment  */
    long dy = x*x, err = dx+dy;                             /* error of 1.step */

    do {
        ret += QPoint(xm-x, ym+y);                            /*   I. Quadrant */
        ret += QPoint(xm+x, ym+y);                            /*  II. Quadrant */
        ret += QPoint(xm+x, ym-y);                            /* III. Quadrant */
        ret += QPoint(xm-x, ym-y);                            /*  IV. Quadrant */
        e2 = 2*err;
        if (e2 >= dx) { x++; err += dx += 2*(long)b*b; }             /* x step */
        if (e2 <= dy) { y++; err += dy += 2*(long)a*a; }             /* y step */
    } while (x <= 0);

    while (y++ < b) {            /* too early stop for flat ellipses with a=1, */
        ret += QPoint(xm, ym+y);                   /* -> finish tip of ellipse */
        ret += QPoint(xm, ym-y);
    }

    return ret;
}

/**
 * Returns an elliptical region based on a rectangle given by x0,y0 (top-left)
 * and x1,y1 (bottom-right), inclusive.
 *
 * From "Bresenham Curve Rasterizing Algorithms", adjusted to output a filled
 * region instead of an outline.
 *
 * @version V20.15 april 2020
 * @copyright MIT open-source license software
 * @url https://github.com/zingl/Bresenham
 * @author  Zingl Alois
 */
QRegion ellipseRegion(int x0, int y0, int x1, int y1)
{
    QRegion ret;

    auto addRect = [&ret](int x0, int y0, int x1, int y1) {
        ret += QRect(QPoint(x0, y0), QPoint(x1, y1));
    };

    long a = abs(x1-x0), b = abs(y1-y0), b1 = b&1;                 /* diameter */
    double dx = 4*(1.0-a)*b*b, dy = 4*(b1+1)*a*a;           /* error increment */
    double err = dx+dy+b1*a*a, e2;                          /* error of 1.step */

    if (x0 > x1) { x0 = x1; x1 += a; }        /* if called with swapped points */
    if (y0 > y1) y0 = y1;                                  /* .. exchange them */
    y0 += (b+1)/2; y1 = y0-b1;                               /* starting pixel */
    a = 8*a*a; b1 = 8*b*b;

    do {
       // (x1, y0)                                            /*   I. Quadrant */
       // (x0, y0)                                            /*  II. Quadrant */
       // (x0, y1)                                            /* III. Quadrant */
       // (x1, y1)                                            /*  IV. Quadrant */

       addRect(x0, y0, x1, y0);                                 /* Bottom half */
       addRect(x0, y1, x1, y1);                                    /* Top half */

       e2 = 2*err;
       if (e2 <= dy) { y0++; y1--; err += dy += a; }                 /* y step */
       if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; }  /* x step */
    } while (x0 <= x1);

    while (y0-y1 <= b) {                /* too early stop of flat ellipses a=1 */
       addRect(x0-1, y0, x1+1, y0);                /* -> finish tip of ellipse */
       addRect(x0-1, y1, x1+1, y1);
       y0++;
       y1--;
    }

    return ret;
}

QRegion ellipseRegion(QRect rect)
{
    // Check for empty rectangle explicitly, because ellipseRegion above can't
    // handle empty rectangles due to the coordinates being inclusive.
    if (rect.width() == 0 || rect.height() == 0)
        return QRegion();

    rect = rect.normalized();
    return ellipseRegion(rect.left(), rect.top(), rect.right(), rect.bottom());
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
