/*
 * tileseteditor.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include "clipboardmanager.h"
#include "editor.h"
#include "wangset.h"

#include <QHash>
#include <QList>
#include <QUrl>

class QAction;
class QComboBox;
class QMainWindow;
class QStackedWidget;
class QToolBar;

namespace Tiled {

class Terrain;
class Tile;
class Tileset;

namespace Internal {

class PropertiesDock;
class TerrainDock;
class TileAnimationEditor;
class TileCollisionDock;
class TilesetDocument;
class TilesetEditorWindow;
class TilesetView;
class UndoDock;
class WangDock;
class Zoomable;

class TilesetEditor : public Editor
{
    Q_OBJECT

public:
    explicit TilesetEditor(QObject *parent = nullptr);

    void saveState() override;
    void restoreState() override;

    void addDocument(Document *document) override;
    void removeDocument(Document *document) override;

    void setCurrentDocument(Document *document) override;
    Document *currentDocument() const override;

    QWidget *editorWidget() const override;

    QList<QToolBar *> toolBars() const override;
    QList<QDockWidget *> dockWidgets() const override;

    StandardActions enabledStandardActions() const override;
    void performStandardAction(StandardAction action) override;

    TilesetView *currentTilesetView() const;
    Tileset *currentTileset() const;
    Zoomable *zoomable() const override;

    QAction *addTilesAction() const;
    QAction *removeTilesAction() const;
    QAction *editTerrainAction() const;
    QAction *editCollisionAction() const;
    QAction *showAnimationEditor() const;

    TileAnimationEditor *tileAnimationEditor() const;

signals:
    void currentTileChanged(Tile *tile);

private slots:
    void currentWidgetChanged();

    void selectionChanged();
    void currentChanged(const QModelIndex &index);
    void indexPressed(const QModelIndex &index);

    void tilesetChanged();
    void updateTilesetView(Tileset *tileset);

    void openAddTilesDialog();
    void addTiles(const QList<QUrl> &urls);
    void removeTiles();

    void setEditTerrain(bool editTerrain);
    void currentTerrainChanged(const Terrain *terrain);

    void setEditCollision(bool editCollision);

    void setEditWang(bool editWang);

    void updateAddRemoveActions();

    void addTerrainType();
    void removeTerrainType();
    void setTerrainImage(Tile *tile);

    void currentWangSetChanged(WangSet *wangSet);
    void currentWangIdChanged(WangId wangId);
    void wangColorChanged(int color, bool edge);
    void addWangSet();
    void removeWangSet();
    void setWangSetImage(Tile *tile);
    void setWangColorImage(Tile *tile, bool isEdge, int index);
    void setWangColorColor(const QColor &color, bool isEdge, int index);

    void onAnimationEditorClosed();

private:
    void setCurrentTile(Tile *tile);

    void retranslateUi();

    TilesetEditorWindow *mMainWindow;
    QToolBar *mMainToolBar;
    QStackedWidget *mWidgetStack;
    QToolBar *mTilesetToolBar;

    QAction *mAddTiles;
    QAction *mRemoveTiles;
    QAction *mShowAnimationEditor;

    PropertiesDock *mPropertiesDock;
    UndoDock *mUndoDock;
    TerrainDock *mTerrainDock;
    TileCollisionDock *mTileCollisionDock;
    WangDock *mWangDock;
    QComboBox *mZoomComboBox;
    TileAnimationEditor *mTileAnimationEditor;

    QHash<TilesetDocument*, TilesetView*> mViewForTileset;
    TilesetDocument *mCurrentTilesetDocument;

    Tile *mCurrentTile;
};

inline QAction *TilesetEditor::addTilesAction() const
{
    return mAddTiles;
}

inline QAction *TilesetEditor::removeTilesAction() const
{
    return mRemoveTiles;
}

inline QAction *TilesetEditor::showAnimationEditor() const
{
    return mShowAnimationEditor;
}

inline TileAnimationEditor *TilesetEditor::tileAnimationEditor() const
{
    return mTileAnimationEditor;
}

} // namespace Internal
} // namespace Tiled
