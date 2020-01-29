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
#include "mapitem.h"

#include <QColor>
#include <QGraphicsScene>
#include <QHash>

namespace Tiled {

class Layer;
class MapObject;
class ObjectGroup;
class Tile;
class TileLayer;
class Tileset;

class AbstractTool;
class LayerItem;
class MapDocument;
class MapObjectItem;
class MapScene;
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

    void setShowTileCollisionShapes(bool enabled);

    QRectF mapBoundingRect() const;

    void setSelectedTool(AbstractTool *tool);

signals:
    void mapDocumentChanged(MapDocument *mapDocument);

protected:
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

private:
    void refreshScene();

    void mapChanged();
    void repaintTileset(Tileset *tileset);

    void tilesetReplaced(int index, Tileset *tileset, Tileset *oldTileset);

    void updateDefaultBackgroundColor();
    void updateSceneRect();

    MapItem *takeOrCreateMapItem(const MapDocumentPtr &mapDocument,
                                 MapItem::DisplayMode displayMode);

    bool eventFilter(QObject *object, QEvent *event) override;

    MapDocument *mMapDocument = nullptr;
    QHash<MapDocument*, MapItem*> mMapItems;
    AbstractTool *mSelectedTool = nullptr;
    bool mUnderMouse = false;
    bool mShowTileCollisionShapes = false;
    Qt::KeyboardModifiers mCurrentModifiers = Qt::NoModifier;
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

} // namespace Tiled
