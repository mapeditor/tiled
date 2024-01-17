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
#include "session.h"

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
class DebugDrawItem;
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
    void setParallaxEnabled(bool enabled);
    void setPainterScale(qreal painterScale);
    void setSuppressMouseMoveEvents(bool suppress);

    QRectF mapBoundingRect() const;

    void setSelectedTool(AbstractTool *tool);

    MapItem *mapItem(MapDocument *mapDocument) const;

    DebugDrawItem *debugDrawItem() const;

    const QRectF &viewRect() const;
    void setViewRect(const QRectF &rect);

    void setOverrideBackgroundColor(QColor backgroundColor);

    QPointF absolutePositionForLayer(const Layer &layer) const;
    QPointF layerItemPosition(const Layer &layer) const;
    QPointF parallaxOffset(const Layer &layer) const;

    static SessionOption<bool> enableWorlds;

signals:
    void mapDocumentChanged(MapDocument *mapDocument);

    void sceneRefreshed();

    void fontChanged();
    void parallaxParametersChanged();

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

    void changeEvent(const ChangeEvent &change);
    void mapChanged();
    void repaintTileset(Tileset *tileset);

    void tilesetReplaced(int index, Tileset *tileset, Tileset *oldTileset);

    void updateDefaultBackgroundColor();
    void updateBackgroundColor();
    void updateSceneRect();

    void setWorldsEnabled(bool enabled);

    MapItem *takeOrCreateMapItem(const MapDocumentPtr &mapDocument,
                                 MapItem::DisplayMode displayMode);

    bool eventFilter(QObject *object, QEvent *event) override;

    bool toolMouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);

    MapDocument *mMapDocument = nullptr;
    QHash<Map*, MapItem*> mMapItems;
    AbstractTool *mSelectedTool = nullptr;
    DebugDrawItem *mDebugDrawItem = nullptr;
    bool mUnderMouse = false;
    bool mShowTileCollisionShapes = false;
    bool mParallaxEnabled = true;
    bool mWorldsEnabled = true;
    bool mSuppressMouseMoveEvents = false;
    bool mMouseMoveEventSuppressed = false;
    Session::CallbackIterator mEnableWorldsCallback;
    Qt::KeyboardModifiers mToolModifiers = Qt::NoModifier;
    Qt::KeyboardModifiers mLastModifiers = Qt::NoModifier;
    QPointF mLastMousePos;
    QRectF mViewRect;
    QColor mDefaultBackgroundColor;
    QColor mOverrideBackgroundColor;
};

/**
 * Returns the map document this scene is displaying.
 */
inline MapDocument *MapScene::mapDocument() const
{
    return mMapDocument;
}

/**
 * Returns the map item displaying the given map, if any.
 */
inline MapItem *MapScene::mapItem(MapDocument *mapDocument) const
{
    return mapDocument ? mMapItems.value(mapDocument->map()) : nullptr;
}

inline DebugDrawItem *MapScene::debugDrawItem() const
{
    return mDebugDrawItem;
}

inline const QRectF &MapScene::viewRect() const
{
    return mViewRect;
}

} // namespace Tiled
