/*
 * objectselectionitem.h
 * Copyright 2015-2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "changeevents.h"

#include <QGraphicsObject>
#include <QHash>

#include <memory>

namespace Tiled {

class GroupLayer;
class Layer;
class MapObject;
class Tile;
class Tileset;

class MapDocument;
class MapObjectItem;
class MapObjectLabel;
class MapObjectOutline;

/**
 * A graphics item displaying object selection.
 *
 * Apart from selection outlines, it also displays name labels when
 * appropriate.
 */
class ObjectSelectionItem : public QGraphicsObject
{
    Q_OBJECT

public:
    ObjectSelectionItem(MapDocument *mapDocument,
                        QGraphicsItem *parent = nullptr);
    ~ObjectSelectionItem() override;

    // QGraphicsItem interface
    QRectF boundingRect() const override { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}

private:
    void changeEvent(const ChangeEvent &event);
    void selectedObjectsChanged();
    void hoveredMapObjectChanged(MapObject *object, MapObject *previous);
    void mapChanged();
    void layerAdded(Layer *layer);
    void layerAboutToBeRemoved(GroupLayer *parentLayer, int index);
    void layerChanged(Layer *layer);
    void syncOverlayItems(const QList<MapObject *> &objects);
    void updateObjectLabelColors();
    void objectsAdded(const QList<MapObject*> &objects);
    void objectsAboutToBeRemoved(const QList<MapObject*> &objects);
    void tilesetTileOffsetChanged(Tileset *tileset);
    void tileTypeChanged(Tile *tile);

    void objectLabelVisibilityChanged();

    void addRemoveObjectLabels();
    void addRemoveObjectOutlines();

    MapDocument *mMapDocument;
    QHash<MapObject*, MapObjectLabel*> mObjectLabels;
    QHash<MapObject*, MapObjectOutline*> mObjectOutlines;
    std::unique_ptr<MapObjectItem> mHoveredMapObjectItem;
};

} // namespace Tiled
