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

#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "map.h"
#include "maprenderer.h"

#include "mapitem.h"
#include "objectsnode.h"

#include <QtMath>
#include <QQuickWindow>

using namespace Tiled;
using namespace TiledQuick;

namespace {

static inline QSGTexture *tilesetTexture(Tileset *tileset,
                                         QQuickWindow *window)
{
    static QHash<Tileset *, QSGTexture *> cache;

    QSGTexture *texture = cache.value(tileset);
    if (!texture) {
        const QString imagePath(Tiled::urlToLocalFileOrQrc(tileset->imageSource()));
        texture = window->createTextureFromImage(QImage(imagePath));
        cache.insert(tileset, texture);
    }
    return texture;
}

struct TilesetHelper
{
    TilesetHelper(const MapItem *mapItem)
        : mWindow(mapItem->window())
        , mTileset(nullptr)
        , mTexture(nullptr)
        , mMargin(0)
        , mTileHSpace(0)
        , mTileVSpace(0)
        , mTilesPerRow(0)
    {
    }

    Tileset *tileset() const { return mTileset; }
    QSGTexture *texture() const { return mTexture; }

    void setTileset(Tileset *tileset)
    {
        mTileset = tileset;
        mTexture = tilesetTexture(tileset, mWindow);
        if (!mTexture)
            return;

        const int tileSpacing = tileset->tileSpacing();
        mMargin = tileset->margin();
        mTileHSpace = tileset->tileWidth() + tileSpacing;
        mTileVSpace = tileset->tileHeight() + tileSpacing;

        const QSize tilesetSize = mTexture->textureSize();
        const int availableWidth = tilesetSize.width() + tileSpacing - mMargin;
        mTilesPerRow = qMax(availableWidth / mTileHSpace, 1);
    }

    void setTextureCoordinates(ObjectData &data, const Cell &cell) const
    {
        const int tileId = cell.tileId();
        const int column = tileId % mTilesPerRow;
        const int row = tileId / mTilesPerRow;

        data.tx = column * mTileHSpace + mMargin;
        data.ty = row * mTileVSpace + mMargin;
    }

private:
    QQuickWindow *mWindow;
    Tileset *mTileset;
    QSGTexture *mTexture;
    int mMargin;
    int mTileHSpace;
    int mTileVSpace;
    int mTilesPerRow;
};

} // anonymous namespace


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

QSGNode *ObjectGroupItem::updatePaintNode(QSGNode *node,
                                          QQuickItem::UpdatePaintNodeData *)
{
    delete node;
    node = new QSGNode;
    node->setFlag(QSGNode::OwnedByParent);

    TilesetHelper helper(static_cast<MapItem*>(parentItem()));

    QVector<ObjectData> objectData;
    objectData.reserve(ObjectsNode::MaxObjectCount);

    for (auto object : *mGroup) {
        const Cell &cell = object->cell();

        if (cell.tileset() != helper.tileset() || objectData.size() == ObjectsNode::MaxObjectCount) {
            if (!objectData.isEmpty()) {
                node->appendChildNode(new ObjectsNode(helper.texture(), objectData));
                objectData.clear();
            }

            helper.setTileset(cell.tileset());
        }

        ObjectData data;
        data.rotation = object->rotation();
        data.x = object->x() - x();
        data.y = object->y() - y() - object->height();
        data.width = object->width();
        data.height = object->height();
        data.twidth = cell.tile()->height();
        data.theight = cell.tile()->width();
        data.flippedHorizontally = cell.flippedHorizontally();
        data.flippedVertically = cell.flippedVertically();
        data.shape = object->shape();
        data.isTileObject = object->isTileObject();
        data.alpha = object->opacity() * mGroup->opacity() * mGroup->tintColor().alpha();

        if (mGroup->tintColor().isValid()) {
            data.tintR = mGroup->tintColor().red();
            data.tintG = mGroup->tintColor().green();
            data.tintB = mGroup->tintColor().blue();
        } else {
            data.tintR = 255;
            data.tintG = 255;
            data.tintB = 255;
        }

        helper.setTextureCoordinates(data, cell);

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

    MapItem *parent = qobject_cast<MapItem*>(parentItem());
    if (visible) {
        updateVisibleObjects();

        if (parent)
            connect(parent, &MapItem::visibleAreaChanged, this, &ObjectGroupItem::updateVisibleObjects);
    } else {
        if (parent)
            disconnect(parent, &MapItem::visibleAreaChanged, this, &ObjectGroupItem::updateVisibleObjects);
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
        helper.setTextureCoordinates(data[0], mCell);

        node = new ObjectsNode(helper.texture(), data);
    }

    return node;
}
