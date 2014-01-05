/*
 * tileanimationdriver.cpp
 * Copyright 2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tileanimationdriver.h"

namespace Tiled {
namespace Internal {

TileAnimationDriver::TileAnimationDriver(QObject *parent) :
    QAbstractAnimation(parent)
{
    setLoopCount(-1); // loop forever
}

int TileAnimationDriver::duration() const
{
    return 1000;
}

void TileAnimationDriver::updateCurrentTime(int currentTime)
{
    static int lastTime = currentTime;

    int elapsed = currentTime - lastTime;
    if (elapsed < 0)
        elapsed += 1000;

    lastTime = currentTime;

    emit update(elapsed);
}

} // namespace Internal
} // namespace Tiled
