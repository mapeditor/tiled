/*
 * terraindock.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2012, Manu Evans <turkeyman@gmail.com>
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

#ifndef TERRAINDOCK_H
#define TERRAINDOCK_H

#include <QDockWidget>
#include <QMap>

class QAction;
class QMenu;
class QTabBar;
class QToolBar;
class QToolButton;
class QSignalMapper;
class QStackedWidget;

namespace Tiled {

class Tile;
class TileLayer;
class Tileset;
class Terrain;

namespace Internal {

class MapDocument;
class TerrainView;

/**
 * The dock widget that displays the tilesets. Also keeps track of the
 * currently selected tile.
 */
class TerrainDock : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    TerrainDock(QWidget *parent = 0);

    ~TerrainDock();

    /**
     * Sets the map for which the tilesets should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Returns the currently selected tile.
     */
    Terrain *currentTerrain() const { return mCurrentTerrain; }

signals:
    /**
     * Emitted when the current tile changed.
     */
    void currentTerrainChanged(const Terrain *terrain);

    /**
     * Emitted when files are dropped at the tileset dock.
     */
    void tilesetsDropped(const QStringList &paths);

protected:
    void changeEvent(QEvent *e);

    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private slots:
    void insertTilesetView(int index, Tileset *tileset);
    void updateActions();
    void updateCurrentTiles();
    void tilesetChanged(Tileset *tileset);
    void tilesetRemoved(Tileset *tileset);
    void tilesetMoved(int from, int to);
    void tilesetNameChanged(Tileset *tileset);

    void removeTileset();
    void removeTileset(int index);
    void moveTileset(int from, int to);

    void documentCloseRequested(int index);

    void refreshTerrainMenu();

private:
    void setCurrentTerrain(Terrain *terrain);
    void retranslateUi();

    Tileset *currentTileset() const;
    TerrainView *terrainViewAt(int index) const;

    MapDocument *mMapDocument;
    QTabBar *mTabBar;
    QStackedWidget *mViewStack;
    Terrain *mCurrentTerrain;

    QMap<MapDocument *, QString> mCurrentTilesets;

    QToolButton *mTerrainMenuButton;
    QMenu *mTerrainMenu; //opens on click of mTerraintMenu
    QSignalMapper *mTerrainMenuMapper; //needed due to dynamic content
};

} // namespace Internal
} // namespace Tiled

#endif // TERRAINDOCK_H
