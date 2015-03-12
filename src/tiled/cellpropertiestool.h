/*
 * cellpropertiestool.h
 * Copyright 2011, Dan Danese <biouxtai@danese.us>
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

#ifndef CELLPROPERTIESTOOL_H
#define CELLPROPERTIESTOOL_H

#include "abstracttiletool.h"
#include "mapdocument.h"

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * Implements a tile selector that will edit a cell's properties.
 */
class CellPropertiesTool : public AbstractTileTool
{
    Q_OBJECT

public:
    CellPropertiesTool(QObject *parent = 0);
    ~CellPropertiesTool();

    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);
	void languageChanged();
	void tilePositionChanged(const QPoint &tilePos);
    void setMapDocument(MapDocument *mapDocument);

private:
    MapDocument *mMapDocument;
};

} // namespace Internal
} // namespace Tiled


#endif //CELLPROPERTIESTOOL_H
