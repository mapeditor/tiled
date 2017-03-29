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

#include "hex.h"

using namespace Tiled;

Hex::Hex(int x, int y, int z):
    mX(x),
    mY(y),
    mZ(z) { }

Hex::Hex(QPoint point,
         Map::StaggerIndex staggerIndex,
         Map::StaggerAxis staggerAxis)
{
    Hex(point.x(),point.y(),staggerIndex,staggerAxis);
}

Hex::Hex(int col, int row,
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

QPoint Hex::toStagger(Map::StaggerIndex staggerIndex,
                      Map::StaggerAxis staggerAxis)
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
    if(direction == RotateLeft) {
        mX = -mY;
        mY = -mZ;
        mZ = -tX;
    } else {
        mX = -mZ;
        mZ = -mY;
        mY = -tX;
    }
}

Hex Hex::operator +(const Hex& h)
{
    return Hex(mX + h.x(), mY + h.y(), mZ + h.z());
}

Hex Hex::operator -(const Hex& h)
{
    return Hex(mX - h.x(), mY - h.y(), mZ - h.z());
}

Hex Hex::operator *(const float& f)
{
    return Hex(mX*f, mY*f, mZ*f);
}

Hex Hex::operator /(const float& f)
{
    return Hex(mX/f, mY/f, mZ/f);
}

void Hex::operator +=(const Hex& h)
{
    mX += h.x();
    mY += h.y();
    mZ += h.z();
}

void Hex::operator -=(const Hex& h)
{
    mX -= h.x();
    mY -= h.y();
    mZ -= h.z();
}
