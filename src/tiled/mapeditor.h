/*
 * mapeditor.h
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

#include <QHash>
#include <QMap>
#include <QVariant>

#include "clipboardmanager.h"
#include "editor.h"
#include "tiled.h"
#include "tileset.h"

class QAction;
class QComboBox;
class QLabel;
class QMainWindow;
class QStackedWidget;
class QToolBar;
class QToolButton;

namespace Tiled {

class Terrain;

class AbstractTool;
class BucketFillTool;
class ComboBoxProxyModel;
class EditableMap;
class EditPolygonTool;
class LayerDock;
class MapDocument;
class MapView;
class MapsDock;
class MiniMapDock;
class ObjectsDock;
class PropertiesDock;
class ReversingProxyModel;
class ShapeFillTool;
class StampBrush;
class TemplatesDock;
class TerrainBrush;
class TerrainDock;
class TileStamp;
class TileStampManager;
class TileStampsDock;
class TilesetDock;
class ToolManager;
class TreeViewComboBox;
class UndoDock;
class WangBrush;
class WangDock;
class Zoomable;

class MapEditor : public Editor
{
    Q_OBJECT

    Q_PROPERTY(Tiled::TilesetDock *tilesetsView READ tilesetDock)
    Q_PROPERTY(Tiled::EditableMap *currentBrush READ currentBrush WRITE setCurrentBrush)
    Q_PROPERTY(Tiled::MapView *currentMapView READ currentMapView)

public:
    explicit MapEditor(QObject *parent = nullptr);
    ~MapEditor() override;

    TilesetDock *tilesetDock() const { return mTilesetDock; }

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

    void resetLayout() override;

    MapView *viewForDocument(MapDocument *mapDocument) const;
    MapView *currentMapView() const;
    Zoomable *zoomable() const override;

    void saveDocumentState(MapDocument *mapDocument);

    void showMessage(const QString &text, int timeout = 0);

    void setCurrentTileset(const SharedTileset &tileset);
    SharedTileset currentTileset() const;

    EditableMap *currentBrush() const;
    void setCurrentBrush(EditableMap *editableMap);

    void addExternalTilesets(const QStringList &fileNames);

    QAction *actionSelectNextTileset() const;
    QAction *actionSelectPreviousTileset() const;

private:
    void setSelectedTool(AbstractTool *tool);

    void paste(ClipboardManager::PasteFlags flags);

    void setRandom(bool value);
    void setWangFill(bool value);

    void setStamp(const TileStamp &stamp);
    void selectTerrainBrush();

    void selectWangBrush();

    void filesDroppedOnTilesetDock(const QStringList &fileNames);

    void currentWidgetChanged();

    void cursorChanged(const QCursor &cursor);

    void updateStatusInfoLabel(const QString &statusInfo);

    void layerComboActivated();
    void updateLayerComboIndex();

    void setupQuickStamps();
    void retranslateUi();
    void showTileCollisionShapesChanged(bool enabled);

    void handleExternalTilesetsAndImages(const QStringList &fileNames,
                                         bool handleImages);

    SharedTileset newTileset(const QString &fileName, const QImage &image);

    QMainWindow *mMainWindow;

    LayerDock *mLayerDock;
    QStackedWidget *mWidgetStack;
    QHash<MapDocument*, MapView*> mWidgetForMap;
    MapDocument *mCurrentMapDocument;

    PropertiesDock *mPropertiesDock;
    MapsDock *mMapsDock;
    UndoDock *mUndoDock;
    ObjectsDock *mObjectsDock;
    TemplatesDock *mTemplatesDock;
    TilesetDock *mTilesetDock;
    TerrainDock *mTerrainDock;
    WangDock *mWangDock;
    MiniMapDock* mMiniMapDock;
    TileStampsDock *mTileStampsDock;

    TreeViewComboBox *mLayerComboBox;
    ComboBoxProxyModel *mComboBoxProxyModel;
    ReversingProxyModel *mReversingProxyModel;

    Zoomable *mZoomable;
    QComboBox *mZoomComboBox;
    QLabel *mStatusInfoLabel;

    StampBrush *mStampBrush;
    BucketFillTool *mBucketFillTool;
    ShapeFillTool *mShapeFillTool;
    TerrainBrush *mTerrainBrush;
    WangBrush *mWangBrush;
    EditPolygonTool *mEditPolygonTool;

    QToolBar *mMainToolBar;
    QToolBar *mToolsToolBar;
    QToolBar *mToolSpecificToolBar;
    ToolManager *mToolManager;
    AbstractTool *mSelectedTool;
    MapView *mViewWithTool;

    TileStampManager *mTileStampManager;

    QVariantMap mMapStates;
};


inline MapView *MapEditor::viewForDocument(MapDocument *mapDocument) const
{
    return mWidgetForMap.value(mapDocument);
}

inline MapView *MapEditor::currentMapView() const
{
    return viewForDocument(mCurrentMapDocument);
}

} // namespace Tiled
