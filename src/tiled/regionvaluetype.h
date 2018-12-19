/*
 * regionvaluetype.h
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

#pragma once

#include <QObject>
#include <QRegion>

namespace Tiled {

class RegionValueType
{
    Q_GADGET

    Q_PROPERTY(QRect boundingRect READ boundingRect)

public:
    RegionValueType();
    RegionValueType(int x, int y, int w, int h);
    explicit RegionValueType(const QRect &rect);
    explicit RegionValueType(const QRegion &region);

    Q_INVOKABLE QString toString() const;

    Q_INVOKABLE void add(const QRect &rect);
    Q_INVOKABLE void add(const QRectF &rect);
    Q_INVOKABLE void add(const Tiled::RegionValueType &region);
    Q_INVOKABLE void subtract(const QRect &rect);
    Q_INVOKABLE void subtract(const QRectF &rect);
    Q_INVOKABLE void subtract(const Tiled::RegionValueType &region);
    Q_INVOKABLE void intersect(const QRect &rect);
    Q_INVOKABLE void intersect(const QRectF &rect);
    Q_INVOKABLE void intersect(const Tiled::RegionValueType &region);

    QRect boundingRect() const;
    const QRegion &region() const;

private:
    QRegion mRegion;
};


inline void RegionValueType::add(const QRect &rect)
{
    mRegion += rect;
}

inline void RegionValueType::add(const QRectF &rect)
{
    add(rect.toRect());
}

inline void RegionValueType::add(const RegionValueType &region)
{
    mRegion += region.region();
}

inline void RegionValueType::subtract(const QRect &rect)
{
    mRegion -= rect;
}

inline void RegionValueType::subtract(const QRectF &rect)
{
    subtract(rect.toRect());
}

inline void RegionValueType::subtract(const RegionValueType &region)
{
    mRegion -= region.region();
}

inline void RegionValueType::intersect(const QRect &rect)
{
    mRegion &= rect;
}

inline void RegionValueType::intersect(const QRectF &rect)
{
    intersect(rect.toRect());
}

inline void RegionValueType::intersect(const RegionValueType &region)
{
    mRegion &= region.region();
}

inline QRect RegionValueType::boundingRect() const
{
    return mRegion.boundingRect();
}

inline const QRegion &RegionValueType::region() const
{
    return mRegion;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::RegionValueType)
