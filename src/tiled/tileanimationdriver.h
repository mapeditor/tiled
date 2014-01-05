/*
 * tileanimationdriver.h
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

#ifndef TILED_INTERNAL_TILEANIMATIONDRIVER_H
#define TILED_INTERNAL_TILEANIMATIONDRIVER_H

#include <QAbstractAnimation>

namespace Tiled {

class Tileset;

namespace Internal {

class TileAnimationDriver : public QAbstractAnimation
{
    Q_OBJECT

public:
    explicit TileAnimationDriver(QObject *parent = 0);

    int duration() const;

signals:
    /**
     * Emitted every time a logic update should be made. \a deltaTime is in
     * milliseconds.
     */
    void update(int deltaTime);

protected:
    void updateCurrentTime(int currentTime);
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_TILEANIMATIONDRIVER_H
