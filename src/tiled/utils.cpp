/*
 * utils.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "utils.h"

#include <QAction>
#include <QCoreApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QMenu>

static QString toImageFileFilter(const QList<QByteArray> &formats)
{
    QString filter(QCoreApplication::translate("Utils", "Image files"));
    filter += QLatin1String(" (");
    bool first = true;
    foreach (const QByteArray &format, formats) {
        if (!first)
            filter += QLatin1Char(' ');
        first = false;
        filter += QLatin1String("*.");
        filter += QString::fromLatin1(format.toLower());
    }
    filter += QLatin1Char(')');
    return filter;
}

/**
 * Checks if a given rectangle \a rect is coherent to another given \a region.
 * 'coherent' means that either the rectangle is overlapping the region or
 * the rectangle contains at least one tile, which is a direct neighbour
 * to a tile, which belongs to the region.
 */
static bool isCoherentTo(const QRect &rect, const QRegion &region)
{
    // check if the region is coherent at top or bottom
    if (region.intersects(rect.adjusted(0, -1, 0, 1)))
        return true;

    // check if the region is coherent at left or right side
    if (region.intersects(rect.adjusted(-1, 0, 1, 0)))
        return true;

    return false;
}

namespace Tiled {
namespace Utils {

QString readableImageFormatsFilter()
{
    return toImageFileFilter(QImageReader::supportedImageFormats());
}

QString writableImageFormatsFilter()
{
    return toImageFileFilter(QImageWriter::supportedImageFormats());
}

QList<QRegion> coherentRegions(const QRegion &region)
{
    QList<QRegion> result;
    QVector<QRect> rects = region.rects();

    while (!rects.isEmpty()) {
        QRegion newCoherentRegion = rects.last();
        rects.pop_back();

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

QPoint getMidPoint(const QRegion &region)
{
    if (region.isEmpty())
        return QPoint();

    int resx = 0;
    int resy = 0;
    int allweight = 0;
    foreach (QRect rect, region.rects()) {
        int weight = rect.width() * rect.height();
        allweight += weight;
        int x = (rect.right() - rect.left()) / 2 + rect.left();
        int y = (rect.top() - rect.bottom()) / 2 + rect.bottom();
        resx += weight * x;
        resy += weight * y;
    }
    return QPoint((int)(resx / allweight), (int)(resy / allweight));
}

} // namespace Utils
} // namespace Tiled
