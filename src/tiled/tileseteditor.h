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

#ifndef TILED_INTERNAL_TILESETEDITOR_H
#define TILED_INTERNAL_TILESETEDITOR_H

#include "editor.h"

#include <QHash>

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
class TilesetDocument;
class TilesetView;
class TileAnimationEditor;
class TileCollisionEditor;
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

    TilesetView *currentTilesetView() const;
    Tileset *currentTileset() const;

    TileAnimationEditor *tileAnimationEditor() const;
    TileCollisionEditor *tileCollisionEditor() const;

signals:
    void currentTileChanged(Tile *tile);

private slots:
    void currentWidgetChanged();

    void selectionChanged();
    void currentChanged(const QModelIndex &index);
    void indexPressed(const QModelIndex &index);

    void tilesetChanged();

    void addTiles();
    void removeTiles();

    void setEditTerrain(bool editTerrain);
    void currentTerrainChanged(const Terrain *terrain);

    void updateAddRemoveActions();

    void addTerrainType();
    void removeTerrainType();
    void setTerrainImage(Tile *tile);

private:
    void setCurrentTile(Tile *tile);

    void retranslateUi();

    QMainWindow *mMainWindow;
    QStackedWidget *mWidgetStack;
    QToolBar *mTilesetToolBar;

    QAction *mAddTiles;
    QAction *mRemoveTiles;
    QAction *mEditTerrain;

    PropertiesDock *mPropertiesDock;
    TerrainDock *mTerrainDock;
    Zoomable *mZoomable;
    QComboBox *mZoomComboBox;
    TileAnimationEditor *mTileAnimationEditor;
    TileCollisionEditor *mTileCollisionEditor;

    QHash<TilesetDocument*, TilesetView*> mViewForTileset;
    TilesetDocument *mCurrentTilesetDocument;

    Tile *mCurrentTile;
};

inline TileAnimationEditor *TilesetEditor::tileAnimationEditor() const
{
    return mTileAnimationEditor;
}

inline TileCollisionEditor *TilesetEditor::tileCollisionEditor() const
{
    return mTileCollisionEditor;
}

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_TILESETEDITOR_H
