/*
 * zoomable.cpp
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

#include "zoomable.h"

using namespace Tiled::Internal;

static const int zoomFactorCount = 10;
static const qreal zoomFactors[zoomFactorCount] = {
    0.0625,
    0.125,
    0.25,
    0.5,
    0.75,
    1.0,
    1.5,
    2.0,
    3.0,
    4.0
};

Zoomable::Zoomable(QObject *parent)
    : QObject(parent)
    , mScale(1)
{
}

void Zoomable::setScale(qreal scale)
{
    if (scale == mScale)
        return;

    mScale = scale;
    emit scaleChanged(mScale);
}

bool Zoomable::canZoomIn() const
{
    return mScale < zoomFactors[zoomFactorCount - 1];
}

bool Zoomable::canZoomOut() const
{
    return mScale > zoomFactors[0];
}

void Zoomable::zoomIn()
{
    for (int i = 0; i < zoomFactorCount; ++i) {
        if (zoomFactors[i] > mScale) {
            setScale(zoomFactors[i]);
            break;
        }
    }
}

void Zoomable::zoomOut()
{
    for (int i = zoomFactorCount - 1; i >= 0; --i) {
        if (zoomFactors[i] < mScale) {
            setScale(zoomFactors[i]);
            break;
        }
    }
}

void Zoomable::resetZoom()
{
    setScale(1);
}
