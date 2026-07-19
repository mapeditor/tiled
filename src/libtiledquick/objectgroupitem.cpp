/*
 * objectgroupitem.cpp
 * Copyright 2026, UltraDagon
 *
 * This file is part of Tiled Quick.
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

#include "objectgroupitem.h"

#include "map.h"
#include "maprenderer.h"

#include "objectsnode.h"
#include "tilesethelper.h"

#include <QtMath>
#include <QQuickWindow>

using namespace Tiled;
using namespace TiledQuick;

ObjectGroupItem::ObjectGroupItem(ObjectGroup *group, MapRenderer *renderer,
                                 MapItem *parent)
    : QQuickItem(parent)
    , mGroup(group)
    , mRenderer(renderer)
    , mVisibleArea(parent->visibleArea())
{
    setFlag(ItemHasContents);
    groupVisibilityChanged();

    syncWithObjectGroup();
    setOpacity(mGroup->opacity());
}

void ObjectGroupItem::syncWithObjectGroup()
{
    const QRectF boundingRect = mGroup->objectsBoundingRect();
    setPosition(boundingRect.topLeft());
    setSize(boundingRect.size());
}

void ObjectGroupItem::setMapScale(const qreal &scale)
{
    mScale = scale;
}

QSGNode *ObjectGroupItem::updatePaintNode(QSGNode *node,
                                          QQuickItem::UpdatePaintNodeData *)
{
    delete node;
    node = new QSGNode;
    node->setFlag(QSGNode::OwnedByParent);

    TilesetHelper helper = TilesetHelper::instance(static_cast<MapItem*>(parentItem()));

    QVector<ObjectData> objectData;

    for (auto object : *mGroup) {
        const Cell &cell = object->cell();

        if (cell.tileset() != helper.tileset()) {
            if (!objectData.isEmpty()) {
                node->appendChildNode(new ObjectsNode(helper.texture(), objectData));
                objectData.clear();
            }

            if (!cell.isEmpty())
                helper.setTileset(cell.tileset());
        }

        ObjectData data;

        switch (object->shape())
        {
        case MapObject::Shape::Rectangle:
            if (object->isTileObject())
                data.type = ObjectGroupMaterial::ObjectType::Tile;
            else
                data.type = ObjectGroupMaterial::ObjectType::Rectangle;
            break;
        case MapObject::Shape::Polygon:
            data.type = ObjectGroupMaterial::ObjectType::Polygon;
            break;
        case MapObject::Shape::Polyline:
            data.type = ObjectGroupMaterial::ObjectType::Polyline;
            break;
        case MapObject::Shape::Ellipse:
            data.type = ObjectGroupMaterial::ObjectType::Ellipse;
            break;
        case MapObject::Shape::Capsule:
            data.type = ObjectGroupMaterial::ObjectType::Capsule;
            break;
        case MapObject::Shape::Text:
            data.type = ObjectGroupMaterial::ObjectType::Text;
            break;
        case MapObject::Shape::Point:
            data.type = ObjectGroupMaterial::ObjectType::Point;
            break;
        }

        data.rotation = object->rotation();
        data.x = object->x() - x();
        data.y = object->y() - y() - ((data.type == ObjectGroupMaterial::ObjectType::Tile) ? object->height() : 0);
        data.width = object->width();
        data.height = object->height();

        if (!cell.isEmpty()) {
            data.twidth = cell.tile()->width();
            data.theight = cell.tile()->height();
            data.flippedHorizontally = cell.flippedHorizontally();
            data.flippedVertically = cell.flippedVertically();
        } else {
            data.twidth = 0;
            data.theight = 0;
            data.flippedHorizontally = false;
            data.flippedVertically = false;
        }

        data.alpha = object->opacity() * mGroup->tintColor().alpha();

        if (mGroup->tintColor().isValid()) {
            data.tint_r = mGroup->tintColor().red();
            data.tint_g = mGroup->tintColor().green();
            data.tint_b = mGroup->tintColor().blue();
        } else {
            data.tint_r = 255;
            data.tint_g = 255;
            data.tint_b = 255;
        }

        helper.setTextureCoordinates(data.tx, data.ty, cell);

        objectData.append(data);
    }

    if (!objectData.isEmpty()) {
        node->appendChildNode(new ObjectsNode(helper.texture(), objectData));
    }

    return node;
}

void ObjectGroupItem::updateVisibleObjects()
{
    update();
}

void ObjectGroupItem::groupVisibilityChanged()
{
    const bool visible = mGroup->isVisible();
    setVisible(visible);

    // MapItem *parent = qobject_cast<MapItem*>(parentItem());
    if (visible) {
        updateVisibleObjects();

        // if (parent)
        //     connect(parent, &MapItem::visibleAreaChanged, this, &ObjectGroupItem::updateVisibleObjects);
    } else {
        // if (parent)
        //     disconnect(parent, &MapItem::visibleAreaChanged, this, &ObjectGroupItem::updateVisibleObjects);
    }
}


ObjectItem::ObjectItem(const Cell &cell, QPoint position, MapItem *parent)
    : QQuickItem(parent)
    , mCell(cell)
    , mPosition(position)
{
    setFlag(ItemHasContents);
    setZ(position.y() * parent->map()->tileHeight());
}

QSGNode *ObjectItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    if (!node) {
        const MapItem *mapItem = static_cast<MapItem*>(parent());

        TilesetHelper helper(mapItem);
        Tileset *tileset = mCell.tileset();
        helper.setTileset(tileset);

        if (!helper.texture())
            return nullptr;

        Tile *tile = mCell.tile();
        if (!tile)
            return nullptr;

        const QSize tileSize = mapItem->map()->map()->tileSize();
        const QSize size = tile->size();
        const QPoint offset = tileset->tileOffset();

        QVector<ObjectData> data(1);
        data[0].x = mPosition.x() * tileSize.width() + offset.x();
        data[0].y = (mPosition.y() + 1) * tileSize.height() - tileset->tileHeight() + offset.y();
        data[0].width = size.width();
        data[0].height = size.height();
        helper.setTextureCoordinates(data[0].ty, data[0].ty, mCell);

        node = new ObjectsNode(helper.texture(), data);
    }

    return node;
}
