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

#include <memory>

class QAction;
class QComboBox;
class QLabel;
class QMainWindow;
class QStackedWidget;
class QToolBar;
class QToolButton;

namespace Tiled {

class AbstractTool;
class BucketFillTool;
class ComboBoxProxyModel;
class EditPolygonTool;
class EditableMap;
class EditableWangSet;
class LayerDock;
class MapDocument;
class MapView;
class MiniMapDock;
class ObjectsDock;
class PropertiesDock;
class ReversingProxyModel;
class ShapeFillTool;
class StampBrush;
class TemplatesDock;
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

class MapEditor final : public Editor
{
    Q_OBJECT

    Q_PROPERTY(Tiled::TilesetDock *tilesetsView READ tilesetDock CONSTANT)
    Q_PROPERTY(Tiled::EditableMap *currentBrush READ currentBrush WRITE setCurrentBrush)
    Q_PROPERTY(Tiled::EditableWangSet *currentWangSet READ currentWangSet NOTIFY currentWangSetChanged)
    Q_PROPERTY(int currentWangColorIndex READ currentWangColorIndex NOTIFY currentWangColorIndexChanged)
    Q_PROPERTY(Tiled::MapView *currentMapView READ currentMapView CONSTANT)

public:
    explicit MapEditor(QObject *parent = nullptr);
    ~MapEditor() override;

    TilesetDock *tilesetDock() const { return mTilesetDock; }
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

    MapView *viewForDocument(MapDocument *mapDocument) const;
    MapView *currentMapView() const;
    Zoomable *zoomable() const override;

    void saveDocumentState(MapDocument *mapDocument) const;
    void restoreDocumentState(MapDocument *mapDocument) const;

    void setCurrentTileset(const SharedTileset &tileset);
    SharedTileset currentTileset() const;

    EditableMap *currentBrush() const;
    void setCurrentBrush(EditableMap *editableMap);

    EditableWangSet *currentWangSet() const;
    int currentWangColorIndex() const;

    void addExternalTilesets(const QStringList &fileNames);

    QAction *actionSelectNextTileset() const;
    QAction *actionSelectPreviousTileset() const;

    AbstractTool *selectedTool() const;

signals:
    void currentWangSetChanged();
    void currentWangColorIndexChanged(int colorIndex);

private:
    void setSelectedTool(AbstractTool *tool);
    void currentDocumentChanged(Document *document);
    void updateActiveUndoStack();

    void paste(ClipboardManager::PasteFlags flags);

    void setRandom(bool value);
    void setWangFill(bool value);

    void setStamp(const TileStamp &stamp);

    void selectWangBrush();

    void filesDroppedOnTilesetDock(const QStringList &fileNames);

    void currentWidgetChanged();

    void cursorChanged(const QCursor &cursor);

    void updateStatusInfoLabel(const QString &statusInfo);

    void layerComboActivated();
    void updateLayerComboIndex();

    void setupQuickStamps();
    void setUseOpenGL(bool useOpenGL);
    void retranslateUi();
    void showTileCollisionShapesChanged(bool enabled);
    void parallaxEnabledChanged(bool enabled);

    void handleExternalTilesetsAndImages(const QStringList &fileNames,
                                         bool handleImages);

    SharedTileset newTileset(const QString &fileName, const QImage &image);

    QMainWindow *mMainWindow;

    LayerDock *mLayerDock;
    QStackedWidget *mWidgetStack;
    QHash<MapDocument*, MapView*> mWidgetForMap;
    MapDocument *mCurrentMapDocument;

    PropertiesDock *mPropertiesDock;
    UndoDock *mUndoDock;
    ObjectsDock *mObjectsDock;
    TemplatesDock *mTemplatesDock;
    TilesetDock *mTilesetDock;
    WangDock *mWangDock;
    MiniMapDock* mMiniMapDock;
    TileStampsDock *mTileStampsDock;

    std::unique_ptr<TreeViewComboBox> mLayerComboBox;
    ComboBoxProxyModel *mComboBoxProxyModel;
    ReversingProxyModel *mReversingProxyModel;

    Zoomable *mZoomable;
    std::unique_ptr<QComboBox> mZoomComboBox;
    std::unique_ptr<QLabel> mStatusInfoLabel;

    StampBrush *mStampBrush;
    BucketFillTool *mBucketFillTool;
    ShapeFillTool *mShapeFillTool;
    WangBrush *mWangBrush;
    EditPolygonTool *mEditPolygonTool;

    QToolBar *mMainToolBar;
    QToolBar *mToolsToolBar;
    QToolBar *mToolSpecificToolBar;
    ToolManager *mToolManager;
    AbstractTool *mSelectedTool;
    MapView *mViewWithTool;

    TileStampManager *mTileStampManager;
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
