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
class QLabel;
class QMainWindow;
class QStackedWidget;
class QToolBar;

namespace Tiled {

class Tile;
class Tileset;

class EditableWangSet;
class PropertiesDock;
class TemplatesDock;
class TileAnimationEditor;
class TileCollisionDock;
class TilesetDocument;
class TilesetEditorWindow;
class TilesetView;
class UndoDock;
class WangDock;
class Zoomable;

class TilesetEditor final : public Editor
{
    Q_OBJECT

    Q_PROPERTY(Tiled::TileCollisionDock *collisionEditor READ collisionEditor CONSTANT)
    Q_PROPERTY(Tiled::EditableWangSet *currentWangSet READ currentWangSet NOTIFY currentWangSetChanged)
    Q_PROPERTY(int currentWangColorIndex READ currentWangColorIndex NOTIFY currentWangColorIndexChanged)

public:
    explicit TilesetEditor(QObject *parent = nullptr);

    TemplatesDock *templatesDock() const { return mTemplatesDock; }

    void saveState() override;
    void restoreState() override;

    void addDocument(Document *document) override;
    void removeDocument(Document *document) override;

    void setCurrentDocument(Document *document) override;
    Document *currentDocument() const override;

    QWidget *editorWidget() const override;

    QList<QToolBar *> toolBars() const override;
    QList<QDockWidget *> dockWidgets() const override;
    QList<QWidget*> statusBarWidgets() const override;
    QList<QWidget*> permanentStatusBarWidgets() const override;

    StandardActions enabledStandardActions() const override;
    void performStandardAction(StandardAction action) override;

    void resetLayout() override;

    TilesetView *currentTilesetView() const;
    Tileset *currentTileset() const;
    Zoomable *zoomable() const override;

    QAction *addTilesAction() const;
    QAction *removeTilesAction() const;
    QAction *relocateTilesAction() const;
    QAction *editCollisionAction() const;
    QAction *editWangSetsAction() const;
    QAction *showAnimationEditor() const;

    TileAnimationEditor *tileAnimationEditor() const;
    TileCollisionDock *collisionEditor() const;

    EditableWangSet *currentWangSet() const;
    int currentWangColorIndex() const;

signals:
    void currentTileChanged(Tile *tile);

    void currentWangSetChanged();
    void currentWangColorIndexChanged(int colorIndex);

private:
    void currentWidgetChanged();

    void selectionChanged();
    void currentChanged(const QModelIndex &index);
    void indexPressed(const QModelIndex &index);

    void saveDocumentState(TilesetDocument *tilesetDocument) const;
    void restoreDocumentState(TilesetDocument *tilesetDocument) const;

    void tilesetChanged();
    void selectedTilesChanged();
    void updateTilesetView(Tileset *tileset);

    void openAddTilesDialog();
    void addTiles(const QList<QUrl> &urls);
    void removeTiles();

    void setRelocateTiles(bool relocateTiles);
    void setEditCollision(bool editCollision);
    void hasSelectedCollisionObjectsChanged();

    void setEditWang(bool editWang);

    void updateAddRemoveActions();

    void onCurrentWangSetChanged(WangSet *wangSet);
    void currentWangIdChanged(WangId wangId);
    void wangColorChanged(int color);
    void addWangSet(WangSet::Type type);
    void duplicateWangSet();
    void removeWangSet();
    void setWangSetImage(Tile *tile);
    void setWangColorImage(Tile *tile, int index);
    void setWangColorColor(WangColor *wangColor, const QColor &color);

    void onAnimationEditorClosed();

    void setCurrentTile(Tile *tile);

    void retranslateUi();

    TilesetEditorWindow *mMainWindow;
    QToolBar *mMainToolBar;
    QStackedWidget *mWidgetStack;
    QToolBar *mTilesetToolBar;

    QAction *mAddTiles;
    QAction *mRemoveTiles;
    QAction *mRelocateTiles;
    QAction *mShowAnimationEditor;
    QAction *mDynamicWrappingToggle;

    PropertiesDock *mPropertiesDock;
    UndoDock *mUndoDock;
    TileCollisionDock *mTileCollisionDock;
    TemplatesDock *mTemplatesDock;
    WangDock *mWangDock;
    QComboBox *mZoomComboBox;
    QLabel *mStatusInfoLabel;
    TileAnimationEditor *mTileAnimationEditor;

    QHash<TilesetDocument*, TilesetView*> mViewForTileset;
    TilesetDocument *mCurrentTilesetDocument = nullptr;

    Tile *mCurrentTile = nullptr;
    bool mSettingSelectedTiles = false;
};

inline QAction *TilesetEditor::addTilesAction() const
{
    return mAddTiles;
}

inline QAction *TilesetEditor::removeTilesAction() const
{
    return mRemoveTiles;
}

inline QAction *TilesetEditor::relocateTilesAction() const
{
    return mRelocateTiles;
}

inline QAction *TilesetEditor::showAnimationEditor() const
{
    return mShowAnimationEditor;
}

inline TileAnimationEditor *TilesetEditor::tileAnimationEditor() const
{
    return mTileAnimationEditor;
}

inline TileCollisionDock *TilesetEditor::collisionEditor() const
{
    return mTileCollisionDock;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::TilesetEditor*)
