/*
 * hex.cpp
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

#include "hex.h"

using namespace Tiled;

Hex::Hex(int x, int y, int z):
    mX(x),
    mY(y),
    mZ(z)
{}

Hex::Hex(int col, int row,
         Map::StaggerIndex staggerIndex,
         Map::StaggerAxis staggerAxis)
{
    setStaggered(col, row, staggerIndex, staggerAxis);
}

void Hex::setStaggered(int col, int row,
                       Map::StaggerIndex staggerIndex,
                       Map::StaggerAxis staggerAxis)
{
    if (staggerAxis == Map::StaggerX) {
        if (staggerIndex == Map::StaggerEven) {
            mX = col;
            mZ = row - (col + (col & 1)) / 2;
            mY = -mX - mZ;
        } else {
            mX = col;
            mZ = row - (col - (col & 1)) / 2;
            mY = -mX - mZ;
        }
    } else {
        if (staggerIndex == Map::StaggerEven) {
            mX = col - (row + (row & 1)) / 2;
            mZ = row;
            mY = -mX - mZ;
        } else {
            mX = col - (row - (row & 1)) / 2;
            mZ = row;
            mY = -mX - mZ;
        }
    }
}

QPoint Hex::toStaggered(Map::StaggerIndex staggerIndex,
                        Map::StaggerAxis staggerAxis) const
{
    QPoint point;

    if (staggerAxis == Map::StaggerX) {
        if (staggerIndex == Map::StaggerEven) {
            point.setX(mX);
            point.setY(mZ + (mX + (mX & 1)) / 2);
        } else {
            point.setX(mX);
            point.setY(mZ + (mX - (mX & 1)) / 2);
        }
    } else {
        if (staggerIndex == Map::StaggerEven) {
            point.setX(mX + (mZ + (mZ & 1)) / 2);
            point.setY(mZ);
        } else {
            point.setX(mX + (mZ - (mZ & 1)) / 2);
            point.setY(mZ);
        }
    }

    return point;
}

void Hex::rotate(RotateDirection direction)
{
    int tX = mX;
    if (direction == RotateLeft) {
        mX = -mY;
        mY = -mZ;
        mZ = -tX;
    } else {
        mX = -mZ;
        mZ = -mY;
        mY = -tX;
    }
}

Hex Hex::operator +(Hex h) const
{
    return Hex(mX + h.x(), mY + h.y(), mZ + h.z());
}

Hex Hex::operator -(Hex h) const
{
    return Hex(mX - h.x(), mY - h.y(), mZ - h.z());
}

Hex& Hex::operator +=(Hex h)
{
    mX += h.x();
    mY += h.y();
    mZ += h.z();

    return *this;
}

Hex& Hex::operator -=(Hex h)
{
    mX -= h.x();
    mY -= h.y();
    mZ -= h.z();

    return *this;
}
