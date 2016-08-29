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

#ifndef TILED_INTERNAL_MAPEDITOR_H
#define TILED_INTERNAL_MAPEDITOR_H

#include <QHash>
#include <QMap>
#include <QVariant>

#include "editor.h"
#include "tiled.h"

class QComboBox;
class QLabel;
class QMainWindow;
class QStackedWidget;
class QToolBar;

namespace Tiled {

class Terrain;

namespace Internal {

class AbstractTool;
class MapDocument;
class MapView;
class MapViewContainer;
class ToolManager;
class StampBrush;
class TerrainBrush;
class BucketFillTool;
class TileStamp;
class MapDocumentActionHandler;
class MapsDock;
class MiniMapDock;
class ObjectsDock;
class PropertiesDock;
class TerrainDock;
class TilesetDock;
class Zoomable;
class LayerDock;
class TileStampManager;

class MapEditor : public Editor
{
    Q_OBJECT

public:
    explicit MapEditor(QObject *parent = nullptr);
    ~MapEditor();

    void saveState() override;
    void restoreState() override;

    void addDocument(Document *document) override;
    void removeDocument(Document *document) override;

    void setCurrentDocument(Document *document) override;
    Document *currentDocument() const override;

    QWidget *editorWidget() const override;

    QList<QToolBar *> toolBars() const override;
    QList<QDockWidget *> dockWidgets() const override;

    MapView *viewForDocument(MapDocument *mapDocument) const;
    MapView *currentMapView() const;

    void showMessage(const QString &text, int timeout = 0);

public slots:
    void setSelectedTool(AbstractTool *tool);

    void flipHorizontally() { flip(FlipHorizontally); }
    void flipVertically() { flip(FlipVertically); }
    void rotateLeft() { rotate(RotateLeft); }
    void rotateRight() { rotate(RotateRight); }

    void flip(FlipDirection direction);
    void rotate(RotateDirection direction);

    void setStamp(const TileStamp &stamp);
    void selectTerrainBrush();

protected:
    // todo: consider how to get this event here from the main window
//    void changeEvent(QEvent *event) override;

private slots:
    void currentWidgetChanged();

    void cursorChanged(const QCursor &cursor);

    void updateStatusInfoLabel(const QString &statusInfo);

    void layerComboActivated(int index);
    void updateLayerComboIndex();

private:
    void setupQuickStamps();
    void retranslateUi();

    QMainWindow *mMainWindow;

    LayerDock *mLayerDock;
    QStackedWidget *mWidgetStack;
    QHash<MapDocument*, MapViewContainer*> mWidgetForMap;
    MapDocument *mCurrentMapDocument;

    PropertiesDock *mPropertiesDock;
    MapsDock *mMapsDock;
    ObjectsDock *mObjectsDock;
    TilesetDock *mTilesetDock;
    TerrainDock *mTerrainDock;
    MiniMapDock* mMiniMapDock;
    QDockWidget *mTileStampsDock;
    QComboBox *mLayerComboBox;
    Zoomable *mZoomable;
    QComboBox *mZoomComboBox;
    QLabel *mStatusInfoLabel;

    StampBrush *mStampBrush;
    BucketFillTool *mBucketFillTool;
    TerrainBrush *mTerrainBrush;

    QToolBar *mMainToolBar;
    QToolBar *mToolsToolBar;
    ToolManager *mToolManager;
    AbstractTool *mSelectedTool;
    MapView *mViewWithTool;

    TileStampManager *mTileStampManager;

    QMap<QString, QVariant> mMapStates;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_MAPEDITOR_H
