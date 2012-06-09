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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <QPoint>
#include <QRegion>
#include <QVector>

namespace Tiled {

QVector<QPoint> pointsOnEllipse(int x0, int y0, int x1, int y1);
QVector<QPoint> pointsOnLine(int x0, int y0, int x1, int y1);

QList<QRegion> coherentRegions(const QRegion &region);

} // namespace Tiled

#endif // GEOMETRY_H
