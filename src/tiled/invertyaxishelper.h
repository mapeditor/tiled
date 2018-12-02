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

namespace Tiled{
    namespace Internal {

        class InvertYAxisHelper
        {
        public:
            // Constructors
            InvertYAxisHelper()  {}
            InvertYAxisHelper(Map* m) : target(m) {}
                
            // Inverts Y coordinate in grid
            qreal tileY(qreal y) const
            {
                // Check if Invert Y Axis is set
                if(Preferences::instance()->invertYAxis())
                {
                    return MapDocumentActionHandler::instance()->mapDocument()->map()->height() - 1 - y;
                }
                return y;
            }

            // Inverts Y coordinate in pixels
            qreal pixelY(qreal y) const
            {
                // Obtain the map document
                if(Preferences::instance()->invertYAxis())
                {
                    return ((target->height() + 1) * target->tileHeight()) - y;
                }
                return y;
            }

        private:
            Map* target;
        };

    } // Namespace Internal
} // Namespace Tiled