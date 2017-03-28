/*
 * hex.h
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

#pragma once

#include "tiled.h"
#include "map.h"
#include <QPoint>

namespace Tiled {
namespace Internal {

class Hex
{
public:
    Hex(int x, int y, int z);

    Hex(QPoint staggerPoint,
        Map::StaggerIndex staggerIndex,
        Map::StaggerAxis staggerAxis);

    QPoint toStagger(Map::StaggerIndex staggerIndex,
                     Map::StaggerAxis staggerAxis);

    void rotate(RotateDirection direction);

    int x() const { return mX; }
    int y() const { return mY; }
    int z() const { return mZ; }

    int setX(int x) { mX = x; }
    int setY(int y) { mY = y; }
    int setZ(int z) { mZ = z; }

    Hex operator +(const Hex& h);
    Hex operator -(const Hex& h);
    Hex operator *(const float& f);
    Hex operator /(const float& f);

private:
    int mX;
    int mY;
    int mZ;
};

} // namespace Internal
} // namespace Tiled
