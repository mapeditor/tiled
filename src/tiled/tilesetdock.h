/*
 * tilesetdock.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
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

#ifndef TILESETDOCK_H
#define TILESETDOCK_H

#include <QDockWidget>

class QStackedWidget;
class QTabBar;

namespace Tiled {

class Tile;
class TileLayer;
class Tileset;

namespace Internal {

class MapDocument;
class TilesetView;

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

    /**
     * Returns the currently selected tile.
     */
    Tile *currentTile() const { return mCurrentTile; }

signals:
    /**
     * Emitted when the current tile changed.
     */
    void currentTileChanged(Tile *tile);

    /**
     * Emitted when the currently selected tiles changed.
     */
    void currentTilesChanged(const TileLayer *tiles);

protected:
    void changeEvent(QEvent *e);

private slots:
    void insertTilesetView(int index, Tileset *tileset);
    void updateCurrentTiles();
    void tilesetChanged(Tileset *tileset);
    void tilesetRemoved(Tileset *tileset);
    void tilesetMoved(int from, int to);

    void removeTileset(int index);
    void moveTileset(int from, int to);

private:
    void setCurrentTile(Tile *tile);
    void setCurrentTiles(TileLayer *tiles);
    void retranslateUi();
    TilesetView *tilesetViewAt(int index) const;

    MapDocument *mMapDocument;
    QTabBar *mTabBar;
    QStackedWidget *mViewStack;
    Tile *mCurrentTile;
    TileLayer *mCurrentTiles;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETDOCK_H
