/*
 * tilelayeritem.cpp
 * Copyright 2014, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilelayeritem.h"

#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "map.h"
#include "maprenderer.h"

#include "mapitem.h"
#include "tilesnode.h"

#include <QtMath>
#include <QQuickWindow>

using namespace Tiled;
using namespace TiledQuick;

namespace {

/**
 * Returns the texture of a given tileset, or 0 if the image has not been
 * loaded yet.
 */
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

/**
 * This helper class exists mainly to avoid redoing calculations that only need
 * to be done once per tileset.
 */
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

    void setTextureCoordinates(TileData &data, const Cell &cell) const
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


TileLayerItem::TileLayerItem(TileLayer *layer, MapRenderer *renderer,
                             MapItem *parent)
    : QQuickItem(parent)
    , mLayer(layer)
    , mRenderer(renderer)
    , mVisibleArea(parent->visibleArea())
{
    setFlag(ItemHasContents);
    layerVisibilityChanged();

    syncWithTileLayer();
    setOpacity(mLayer->opacity());
}

void TileLayerItem::syncWithTileLayer()
{
    const QRectF boundingRect = mRenderer->boundingRect(mLayer->rect());
    setPosition(boundingRect.topLeft());
    setSize(boundingRect.size());
}



QSGNode *TileLayerItem::updatePaintNode(QSGNode *node,
                                        QQuickItem::UpdatePaintNodeData *)
{
    delete node;
    node = new QSGNode;
    node->setFlag(QSGNode::OwnedByParent);

    TilesetHelper helper(static_cast<MapItem*>(parentItem()));

    QVector<TileData> tileData;
    tileData.reserve(TilesNode::MaxTileCount);

    /**
     * Draws the tiles by adding nodes to the scene graph. When sequentially
     * drawn tiles are using the same tileset, they will share a single
     * geometry node.
     */
    auto tileRenderFunction = [&](const Cell &cell, const QPointF &pos, const QSizeF &size) {
        Tileset *tileset = cell.tileset();
        if (!tileset)
            return;

        if (tileset != helper.tileset() || tileData.size() == TilesNode::MaxTileCount) {
            if (!tileData.isEmpty()) {
                node->appendChildNode(new TilesNode(helper.texture(), tileData));
                tileData.resize(0);
            }

            helper.setTileset(tileset);
        }

        if (!helper.texture())
            return;

        // todo: render "missing tile" marker
//        if (!cell.tile()) {
//            return;
//        }

        const auto offset = tileset->tileOffset();

        TileData data;
        data.x = static_cast<float>(pos.x()) + offset.x();
        data.y = static_cast<float>(pos.y() - size.height()) + offset.y();
        data.width = static_cast<float>(size.width());
        data.height = static_cast<float>(size.height());
        data.flippedHorizontally = cell.flippedHorizontally();
        data.flippedVertically = cell.flippedVertically();
        helper.setTextureCoordinates(data, cell);
        tileData.append(data);
    };

    mRenderer->drawTileLayer(mLayer, tileRenderFunction, mVisibleArea);

    if (!tileData.isEmpty())
        node->appendChildNode(new TilesNode(helper.texture(), tileData));

    return node;
}

void TileLayerItem::updateVisibleTiles()
{
    const MapItem *mapItem = static_cast<MapItem*>(parentItem());
    const QRectF &rect = mapItem->visibleArea();

    if (mVisibleArea != rect) {
        mVisibleArea = rect;
        update();
    }
}

void TileLayerItem::layerVisibilityChanged()
{
    const bool visible = mLayer->isVisible();
    setVisible(visible);

    MapItem *parent = qobject_cast<MapItem*>(parentItem());
    if (visible) {
        updateVisibleTiles();

        if (parent)
            connect(parent, &MapItem::visibleAreaChanged, this, &TileLayerItem::updateVisibleTiles);
    } else {
        if (parent)
            disconnect(parent, &MapItem::visibleAreaChanged, this, &TileLayerItem::updateVisibleTiles);
    }
}


TileItem::TileItem(const Cell &cell, QPoint position, MapItem *parent)
    : QQuickItem(parent)
    , mCell(cell)
    , mPosition(position)
{
    setFlag(ItemHasContents);
    setZ(position.y() * parent->map().mMap->tileHeight());
}

QSGNode *TileItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
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
            return nullptr;   // todo: render "missing tile" marker

        const Map *map = mapItem->map();
        const int tileWidth = map->tileWidth();
        const int tileHeight = map->tileHeight();

        const QSize size = tile->size();
        const QPoint offset = tileset->tileOffset();

        QVector<TileData> data(1);
        data[0].x = mPosition.x() * tileWidth + offset.x();
        data[0].y = (mPosition.y() + 1) * tileHeight - tileset->tileHeight() + offset.y();
        data[0].width = size.width();
        data[0].height = size.height();
        helper.setTextureCoordinates(data[0], mCell);

        node = new TilesNode(helper.texture(), data);
    }

    return node;
}
