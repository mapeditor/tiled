/*
 *
 * Copyright 2015, Your Name <your.name@domain>
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

#ifndef TILED_IMAGEREFERENCE_H
#define TILED_IMAGEREFERENCE_H

#include <QColor>
#include <QImage>
#include <QString>

namespace Tiled {

class ImageReference
{
public:
    ImageReference()
        : width(0)
        , height(0)
    {}

    QString source;
    QColor transparentColor;
    int width;
    int height;
    QByteArray format;
    QByteArray data;

    QImage create() const;
};

} // namespace Tiled

#endif // TILED_IMAGEREFERENCE_H
