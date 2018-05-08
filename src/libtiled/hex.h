/*
 * hex.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "tiled.h"
#include "map.h"
#include <QPoint>

namespace Tiled {

/**
 * Represents a hex position in cube coordinates.
 */
class Hex
{
public:
    Hex(int x, int y, int z);

    Hex(QPoint point,
        Map::StaggerIndex staggerIndex,
        Map::StaggerAxis staggerAxis)
        : Hex(point.x(), point.y(), staggerIndex, staggerAxis)
    {}

    Hex(int col, int row,
        Map::StaggerIndex staggerIndex,
        Map::StaggerAxis staggerAxis);

    QPoint toStaggered(Map::StaggerIndex staggerIndex,
                       Map::StaggerAxis staggerAxis) const;

    void rotate(RotateDirection direction);

    int x() const { return mX; }
    int y() const { return mY; }
    int z() const { return mZ; }

    void setX(int x) { mX = x; }
    void setY(int y) { mY = y; }
    void setZ(int z) { mZ = z; }

    void setStaggered(int col, int row,
                      Map::StaggerIndex staggerIndex,
                      Map::StaggerAxis staggerAxis);

    Hex operator +(Hex h) const;
    Hex operator -(Hex h) const;
    Hex& operator +=(Hex h);
    Hex& operator -=(Hex h);

private:
    int mX;
    int mY;
    int mZ;
};

} // namespace Tiled
