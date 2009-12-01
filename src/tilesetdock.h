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

class QStackedWidget;
class QTabBar;

namespace Tiled {

class TileLayer;
class Tileset;

namespace Internal {

class MapDocument;

/**
 * The dock widget that displays the tilesets. Also keeps track of the
 * currently selected tile.
 */
class TilesetDock : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    TilesetDock(QWidget *parent = 0);

    ~TilesetDock();

    /**
     * Sets the map for which the tilesets should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

signals:
    /**
     * Emitted when the currently selected tile changed.
     */
    void currentTilesChanged(const TileLayer *tiles);

private slots:
    void addTilesetView(Tileset *tileset);
    void selectionChanged();
    void tilesetChanged(const Tileset *tileset);

private:
    void setCurrentTiles(TileLayer *tiles);

    MapDocument *mMapDocument;
    QTabBar *mTabBar;
    QStackedWidget *mViewStack;
    TileLayer *mCurrentTiles;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETDOCK_H
