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

#include <cmath>

using namespace Tiled::Internal;

static const qreal zoomFactors[] = {
    0.0625,
    0.125,
    0.25,
    0.33,
    0.5,
    0.75,
    1.0,
    1.5,
    2.0,
    3.0,
    4.0
};
const int zoomFactorCount = sizeof(zoomFactors) / sizeof(zoomFactors[0]);

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

void Zoomable::handleWheelDelta(int delta)
{
    if (delta <= -120) {
        zoomOut();
    } else if (delta >= 120) {
        zoomIn();
    } else {
        // We're dealing with a finer-resolution mouse. Allow it to have finer
        // control over the zoom level.
        qreal factor = 1 + 0.3 * qAbs(qreal(delta) / 8 / 15);
        if (delta < 0)
            factor = 1 / factor;

        qreal scale = qBound(zoomFactors[0],
                             mScale * factor,
                             zoomFactors[zoomFactorCount - 1]);

        // Round to at most four digits after the decimal point
        setScale(std::floor(scale * 10000 + 0.5) / 10000);
    }
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
