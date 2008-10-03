/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef TILESETDOCK_H
#define TILESETDOCK_H

#include <QDockWidget>

namespace Tiled {
namespace Internal {

class TilesetModel;
class TilesetView;
class MapDocument;

/**
 * The dock widget that displays the tilesets.
 */
class TilesetDock : public QDockWidget
{
public:
    /**
     * Constructor.
     */
    TilesetDock(QWidget *parent = 0);

    /**
     * Sets the map for which the tilesets should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

private:
    TilesetModel *mTilesetModel;
    TilesetView *mTilesetView;
    MapDocument *mMapDocument;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETDOCK_H
