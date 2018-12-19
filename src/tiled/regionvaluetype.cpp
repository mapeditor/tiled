/*
 * regionvaluetype.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "regionvaluetype.h"

namespace Tiled {

RegionValueType::RegionValueType()
{
}

RegionValueType::RegionValueType(int x, int y, int w, int h)
    : mRegion(x, y, w, h)
{
}

RegionValueType::RegionValueType(const QRect &rect)
    : mRegion(rect)
{
}

RegionValueType::RegionValueType(const QRegion &region)
    : mRegion(region)
{
}

QString RegionValueType::toString() const
{
    switch (mRegion.rectCount()) {
    case 0:
        return QLatin1String("Region(empty)");
    case 1: {
        QRect r = boundingRect();
        return QString::asprintf("Region(x = %d, y = %d, w = %d, h = %d)",
                                 r.x(), r.y(), r.width(), r.height());
    }
    default:
        return QLatin1String("Region(...)");
    }
}

} // namespace Tiled
