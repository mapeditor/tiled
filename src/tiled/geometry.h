/*
 * geometry.h
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

#pragma once

#include <QPoint>
#include <QRegion>
#include <QVector>

namespace Tiled {

QVector<QPoint> pointsOnEllipse(int x0, int y0, int x1, int y1);
QRegion ellipseRegion(int x0, int y0, int x1, int y1);
QVector<QPoint> pointsOnLine(int x0, int y0, int x1, int y1);

inline QVector<QPoint> pointsOnEllipse(QPoint a, QPoint b)
{ return pointsOnEllipse(a.x(), a.y(), b.x(), b.y()); }

inline QVector<QPoint> pointsOnLine(QPoint a, QPoint b)
{ return pointsOnLine(a.x(), a.y(), b.x(), b.y()); }

QVector<QRegion> coherentRegions(const QRegion &region);

QTransform rotateAt(const QPointF &position, qreal rotation);

} // namespace Tiled
