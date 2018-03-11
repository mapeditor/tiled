/*
 * invertycoordinatehelper.cpp
 * Copyright 2018, Anthony Divitto <anthonydivitto@gmail.com>
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

#include "invertycoordinatehelper.h"
#include "preferences.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"

Tiled::Internal::InvertYCoordinateHelper::InvertYCoordinateHelper()
{
}

Tiled::Internal::InvertYCoordinateHelper::~InvertYCoordinateHelper()
{
}

qreal Tiled::Internal::InvertYCoordinateHelper::tileY(qreal y)
{
    auto map = MapDocumentActionHandler::instance()->mapDocument()->map();
    if (Preferences::instance()->invertYCoordinates())
        return map->tileHeight() * (map->height() + 1) - y;
    return y - map->tileHeight();
}

qreal Tiled::Internal::InvertYCoordinateHelper::pixelY(qreal y)
{
    if (Preferences::instance()->invertYCoordinates())
        return MapDocumentActionHandler::instance()->mapDocument()->map()->height() - y - 1;
    return y;
}
