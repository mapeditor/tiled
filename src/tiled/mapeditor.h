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

#include <QMainWindow>
#include <QHash>

#include "tiled.h"

class QStackedWidget;

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

class LayerDock;

class MapEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit MapEditor(QWidget *parent = nullptr);
    ~MapEditor();

    void addMapDocument(MapDocument *mapDocument);
    void removeMapDocument(MapDocument *mapDocument);

    void setCurrentMapDocument(MapDocument *mapDocument);
    MapDocument *currentMapDocument() const;

    MapView *viewForDocument(MapDocument *mapDocument) const;
    MapView *currentMapView() const;

signals:

public slots:
    void setSelectedTool(AbstractTool *tool);

    void flipHorizontally() { flip(FlipHorizontally); }
    void flipVertically() { flip(FlipVertically); }
    void rotateLeft() { rotate(RotateLeft); }
    void rotateRight() { rotate(RotateRight); }

    void flip(FlipDirection direction);
    void rotate(RotateDirection direction);

    void setStamp(const TileStamp &stamp);
    void setTerrainBrush(const Terrain *terrain);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void cursorChanged(const QCursor &cursor);

private:
    void retranslateUi();

    LayerDock *mLayerDock;
    QStackedWidget *mWidgetStack;
    QHash<MapDocument*, MapViewContainer*> mWidgetForMap;
    MapDocument *mCurrentMapDocument;

    StampBrush *mStampBrush;
    BucketFillTool *mBucketFillTool;
    TerrainBrush *mTerrainBrush;

    QToolBar *mToolBar;
    ToolManager *mToolManager;
    AbstractTool *mSelectedTool;
    MapView *mViewWithTool;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_MAPEDITOR_H
