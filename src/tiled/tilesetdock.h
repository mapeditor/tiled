/*
 * tilesetdock.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
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
#include <QList>
#include <QMap>

class QAction;
class QActionGroup;
class QComboBox;
class QMenu;
class QModelIndex;
class QSignalMapper;
class QStackedWidget;
class QTabBar;
class QToolBar;
class QToolButton;

namespace Tiled {

class Terrain;
class Tile;
class TileLayer;
class Tileset;

namespace Internal {

class MapDocument;
class TilesetView;
class Zoomable;

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

    /**
     * Emitted when files are dropped at the tileset dock.
     */
    void tilesetsDropped(const QStringList &paths);

protected:
    void changeEvent(QEvent *e);

    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private slots:
    void selectionChanged();
    void updateActions();
    void updateCurrentTiles();
    void updateCurrentTile();
    void indexPressed(const QModelIndex &index);

    void tilesetAdded(int index, Tileset *tileset);
    void tilesetChanged(Tileset *tileset);
    void tilesetRemoved(Tileset *tileset);
    void tilesetMoved(int from, int to);
    void tilesetNameChanged(Tileset *tileset);
    void tileAnimationChanged(Tile *tile);

    void removeTileset();
    void removeTileset(int index);
    void moveTileset(int from, int to);

    void editTilesetProperties();
    void importTileset();
    void exportTileset();

    void editTerrain();
    void addTiles();
    void removeTiles();

    void documentCloseRequested(int index);

    void refreshTilesetMenu();

private:
    void setCurrentTile(Tile *tile);
    void setCurrentTiles(TileLayer *tiles);
    void retranslateUi();

    Tileset *currentTileset() const;
    TilesetView *currentTilesetView() const;
    TilesetView *tilesetViewAt(int index) const;

    MapDocument *mMapDocument;
    QList<Tileset*> mTilesets;
    QTabBar *mTabBar;
    QStackedWidget *mViewStack;
    QToolBar *mToolBar;
    Tile *mCurrentTile;
    TileLayer *mCurrentTiles;
    const Terrain *mTerrain;

    QAction *mImportTileset;
    QAction *mExportTileset;
    QAction *mPropertiesTileset;
    QAction *mDeleteTileset;
    QAction *mEditTerrain;
    QAction *mAddTiles;
    QAction *mRemoveTiles;

    QMap<MapDocument *, QString> mCurrentTilesets;

    QToolButton *mTilesetMenuButton;
    QMenu *mTilesetMenu; //opens on click of mTilesetMenu
    QActionGroup *mTilesetActionGroup;
    QSignalMapper *mTilesetMenuMapper; //needed due to dynamic content

    Zoomable *mZoomable;
    QComboBox *mZoomComboBox;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETDOCK_H
