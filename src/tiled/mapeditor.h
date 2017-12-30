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

class QComboBox;
class QLabel;
class QMainWindow;
class QStackedWidget;
class QToolBar;
class QToolButton;

namespace Tiled {

class ObjectTemplate;
class Terrain;

namespace Internal {

class AbstractTool;
class BucketFillTool;
class EditPolygonTool;
class LayerDock;
class MapDocument;
class MapDocumentActionHandler;
class MapsDock;
class MapView;
class MiniMapDock;
class ObjectsDock;
class TemplatesDock;
class PropertiesDock;
class ReversingProxyModel;
class ShapeFillTool;
class StampBrush;
class TerrainBrush;
class TerrainDock;
class TilesetDock;
class TileStamp;
class TileStampManager;
class ToolManager;
class TreeViewComboBox;
class ComboBoxProxyModel;
class UndoDock;
class WangBrush;
class WangDock;
class Zoomable;

class MapEditor : public Editor
{
    Q_OBJECT

public:
    explicit MapEditor(QObject *parent = nullptr);
    ~MapEditor() override;

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

    void showMessage(const QString &text, int timeout = 0);

public slots:
    void setSelectedTool(AbstractTool *tool);

    void paste(ClipboardManager::PasteFlags flags);

    void flipHorizontally() { flip(FlipHorizontally); }
    void flipVertically() { flip(FlipVertically); }
    void rotateLeft() { rotate(RotateLeft); }
    void rotateRight() { rotate(RotateRight); }

    void flip(FlipDirection direction);
    void rotate(RotateDirection direction);
    void setRandom(bool value);
    void setWangFill(bool value);

    void setStamp(const TileStamp &stamp);
    void selectTerrainBrush();

    void selectWangBrush();

    void addExternalTilesets(const QStringList &fileNames);
    void filesDroppedOnTilesetDock(const QStringList &fileNames);

    void updateTemplateInstances(const ObjectTemplate *objectTemplate);

private slots:
    void currentWidgetChanged();

    void cursorChanged(const QCursor &cursor);

    void updateStatusInfoLabel(const QString &statusInfo);

    void layerComboActivated();
    void updateLayerComboIndex();

private:
    void setupQuickStamps();
    void retranslateUi();

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
    QDockWidget *mTileStampsDock;

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

} // namespace Internal
} // namespace Tiled
