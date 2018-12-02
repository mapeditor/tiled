/*
 * invertyaxishelper.h
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2018, Adrian Frances <adrianfranceslillo@gmail.com>
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

#include "preferences.h"
#include "mapdocumentactionhandler.h"

class InvertYAxisHelper{
    public:
        InvertYAxisHelper() {}
        ~InvertYAxisHelper() {}
        
        // Inverts Y coordinate in grid
        float getY(float y) const
        {
            // Check if Invert Y Axis is set
            if(Tiled::Internal::Preferences::instance()->invertYAxis())
            {
                return Tiled::Internal::MapDocumentActionHandler::instance()->mapDocument()->map()->height() - 1 - y;
            }
            return y;
        }

        // Inverts Y coordinate in pixels
        float getPixelY(float y) const
        {
            // Obtain the map document
            auto map = Tiled::Internal::MapDocumentActionHandler::instance()->mapDocument()->map();
            if(Tiled::Internal::Preferences::instance()->invertYAxis())
            {
                return ((map->height() + 1) * map->tileHeight()) - y;
            }
            return y;
        }

};