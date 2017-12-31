/*
 * mapscene.h
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "mapdocument.h"

#include <QColor>
#include <QGraphicsScene>
#include <QMap>
#include <QSet>

namespace Tiled {

class Layer;
class MapObject;
class ObjectGroup;
class Tile;
class TileLayer;
class Tileset;

namespace Internal {

class AbstractTool;
class LayerItem;
class MapDocument;
class MapObjectItem;
class MapScene;
class MapItem;
class ObjectGroupItem;

/**
 * A graphics scene that represents the contents of a map.
 */
class MapScene : public QGraphicsScene
{
    Q_OBJECT

public:
    MapScene(QObject *parent);
    ~MapScene() override;

    MapDocument *mapDocument() const;
    void setMapDocument(MapDocument *map);

    void enableSelectedTool();
    void disableSelectedTool();

    void setSelectedTool(AbstractTool *tool);

protected:
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    bool event(QEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;

private slots:
    void setGridVisible(bool visible);

    void refreshScene();

    void currentLayerChanged();

    void mapChanged();
    void repaintTileset(Tileset *tileset);
    void tileLayerChanged(TileLayer *, MapDocument::TileLayerChangeFlags flags);

    void layerChanged(Layer *);

    void adaptToTilesetTileSizeChanges();
    void adaptToTileSizeChanges();

    void tilesetReplaced();

private:
    void updateDefaultBackgroundColor();
    void updateSceneRect();

    bool eventFilter(QObject *object, QEvent *event) override;

    MapDocument *mMapDocument;
    MapItem *mMapItem;
    AbstractTool *mSelectedTool;
    AbstractTool *mActiveTool;
    bool mGridVisible;
    bool mUnderMouse;
    Qt::KeyboardModifiers mCurrentModifiers;
    QPointF mLastMousePos;
    QColor mDefaultBackgroundColor;
};

/**
 * Returns the map document this scene is displaying.
 */
inline MapDocument *MapScene::mapDocument() const
{
    return mMapDocument;
}

} // namespace Internal
} // namespace Tiled
