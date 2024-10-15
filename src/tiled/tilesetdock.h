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

#pragma once

#include "tileset.h"

#include <QAbstractItemModel>
#include <QDockWidget>
#include <QList>
#include <QMap>

#include <memory>

class QAction;
class QActionGroup;
class QComboBox;
class QMenu;
class QModelIndex;
class QStackedWidget;
class QTabBar;
class QToolBar;
class QToolButton;

namespace Tiled {

class Tile;
class TileLayer;
class Tileset;

class Document;
class EditableTileset;
class MapDocument;
class TileStamp;
class TilesetDocument;
class TilesetDocumentsFilterModel;
class TilesetView;
class Zoomable;

/**
 * The dock widget that displays the tilesets. Also keeps track of the
 * currently selected tile.
 */
class TilesetDock : public QDockWidget
{
    Q_OBJECT

    Q_PROPERTY(Tiled::EditableTileset *currentTileset READ currentEditableTileset WRITE setCurrentEditableTileset NOTIFY currentTilesetChanged)
    Q_PROPERTY(QList<QObject*> selectedTiles READ selectedTiles WRITE setSelectedTiles)

public:
    TilesetDock(QWidget *parent = nullptr);
    ~TilesetDock() override;

    /**
     * Sets the map for which the tilesets should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Returns the currently selected tile.
     */
    Tile *currentTile() const { return mCurrentTile; }

    bool hasTileset(const SharedTileset &tileset) const;
    void setCurrentTileset(const SharedTileset &tileset);
    SharedTileset currentTileset() const;
    TilesetDocument *currentTilesetDocument() const;

    void setCurrentEditableTileset(EditableTileset *tileset);
    EditableTileset *currentEditableTileset() const;

    void setSelectedTiles(const QList<QObject*> &tiles);
    QList<QObject*> selectedTiles() const;

    void selectTilesInStamp(const TileStamp &);

    QAction *actionSelectNextTileset() const { return mSelectNextTileset; }
    QAction *actionSelectPreviousTileset() const { return mSelectPreviousTileset; }

signals:
    /**
     * Emitted when the currently selected tileset changed.
     */
    void currentTilesetChanged();

    /**
     * Emitted when the current tile changed.
     */
    void currentTileChanged(Tile *tile);

    /**
     * Emitted when the currently selected tiles changed.
     */
    void stampCaptured(const TileStamp &);

    /**
     * Emitted when files are dropped at the tileset dock.
     */
    // todo: change to QList<QUrl>
    void localFilesDropped(const QStringList &paths);

protected:
    void changeEvent(QEvent *e) override;

    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;

private:
    void onCurrentTilesetChanged();
    void selectionChanged();
    void currentChanged(const QModelIndex &index);
    void restoreCurrentTile();

    void updateActions();
    void updateCurrentTiles();
    void indexPressed(const QModelIndex &index);

    void tilesetChanged(Tileset *tileset);
    void tilesetFileNameChanged(const QString &fileName);

    void replaceTileset();
    void replaceTilesetAt(int index);
    void removeTileset();
    void removeTilesetAt(int index);

    void newTileset();
    void editTileset();
    void embedTileset();
    void exportTileset();

    void refreshTilesetMenu();

    void swapTiles(Tile *tileA, Tile *tileB);

    void selectTiles(const QList<Tile *> &tiles);
    void setCurrentTile(Tile *tile);
    void setCurrentTiles(std::unique_ptr<TileLayer> tiles);

    void retranslateUi();

    void onTilesetRowsInserted(const QModelIndex &parent, int first, int last);
    void onTilesetRowsRemoved(const QModelIndex &parent, int first, int last);
    void onTilesetRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    void onTilesetLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
    void onTilesetDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void onTabMoved(int from, int to);
    void tabContextMenuRequested(const QPoint &pos);

    int indexOfTileset(const Tileset *tileset) const;
    TilesetView *currentTilesetView() const;
    TilesetView *tilesetViewAt(int index) const;

    void createTilesetView(int index, TilesetDocument *tilesetDocument);
    void deleteTilesetView(int index);
    void moveTilesetView(int from, int to);
    void setupTilesetModel(TilesetView *view, TilesetDocument *tilesetDocument);

    MapDocument *mMapDocument = nullptr;

    QList<TilesetDocument *> mTilesetDocuments;
    TilesetDocumentsFilterModel *mTilesetDocumentsFilterModel;

    QTabBar *mTabBar;
    QStackedWidget *mSuperViewStack;
    QStackedWidget *mViewStack;
    QToolBar *mToolBar;
    Tile *mCurrentTile = nullptr;
    std::unique_ptr<TileLayer> mCurrentTiles;

    QAction *mNewTileset;
    QAction *mEmbedTileset;
    QAction *mExportTileset;
    QAction *mEditTileset;
    QAction *mReplaceTileset;
    QAction *mRemoveTileset;
    QAction *mSelectNextTileset;
    QAction *mSelectPreviousTileset;
    QAction *mDynamicWrappingToggle;

    QToolButton *mTilesetMenuButton;
    QMenu *mTilesetMenu; //opens on click of mTilesetMenu
    QActionGroup *mTilesetActionGroup;

    QComboBox *mZoomComboBox;

    bool mEmittingStampCaptured = false;
    bool mSynchronizingSelection = false;
    bool mNoChangeCurrentObject = false;
};

} // namespace Tiled
