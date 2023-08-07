/*
 * geometry.h
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#pragma once

#include <QPoint>
#include <QRegion>
#include <QTransform>
#include <QVector>

namespace Tiled {

QVector<QPoint> pointsOnEllipse(int xm, int ym, int a, int b);
QRegion ellipseRegion(int x0, int y0, int x1, int y1);
QRegion ellipseRegion(QRect rect);
QVector<QPoint> pointsOnLine(int x0, int y0, int x1, int y1, bool manhattan = false);

inline QVector<QPoint> pointsOnEllipse(QPoint center, int radiusX, int radiusY)
{ return pointsOnEllipse(center.x(), center.y(), radiusX, radiusY); }

inline QVector<QPoint> pointsOnLine(QPoint a, QPoint b, bool manhattan = false)
{ return pointsOnLine(a.x(), a.y(), b.x(), b.y(), manhattan); }

QVector<QRegion> coherentRegions(const QRegion &region);

QTransform rotateAt(const QPointF &position, qreal rotation);

} // namespace Tiled
