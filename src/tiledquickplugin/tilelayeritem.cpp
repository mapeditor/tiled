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
        mTilesPerRow = availableWidth / mTileHSpace;
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

/**
 * Draws an orthogonal tile layer by adding nodes to the scene graph. When
 * sequentially drawn tiles are using the same tileset, they will share a
 * single geometry node.
 */
static void drawOrthogonalTileLayer(QSGNode *parent,
                          const MapItem *mapItem,
                          const TileLayer *layer,
                          const QRect &rect)
{
    TilesetHelper helper(mapItem);

    const Map *map = mapItem->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    QVector<TileData> tileData;
    tileData.reserve(TilesNode::MaxTileCount);

    const QRect contentRect = rect.intersected(layer->localBounds());

    for (int y = contentRect.top(); y <= contentRect.bottom(); ++y) {
        for (int x = contentRect.left(); x <= contentRect.right(); ++x) {
            const Cell &cell = layer->cellAt(x, y);
            if (cell.isEmpty())
                continue;

            Tileset *tileset = cell.tileset();

            if (tileset != helper.tileset() || tileData.size() == TilesNode::MaxTileCount) {
                if (!tileData.isEmpty()) {
                    parent->appendChildNode(new TilesNode(helper.texture(),
                                                          tileData));
                    tileData.resize(0);
                }

                helper.setTileset(tileset);
            }

            if (!helper.texture())
                continue;

            Tile *tile = cell.tile();
            if (!tile)
                continue;   // todo: render "missing tile" marker

            const QSize size = tile->size();
            const QPoint offset = tileset->tileOffset();

            TileData data;
            data.x = x * tileWidth + offset.x();
            data.y = (y + 1) * tileHeight - tileset->tileHeight() + offset.y();
            data.width = size.width();
            data.height = size.height();
            data.flippedHorizontally = cell.flippedHorizontally();
            data.flippedVertically = cell.flippedVertically();
            helper.setTextureCoordinates(data, cell);
            tileData.append(data);
        }
    }

    if (!tileData.isEmpty())
        parent->appendChildNode(new TilesNode(helper.texture(), tileData));
}

// Avoids passing lots of variables to static free functions.
class IsometricRenderHelper
{
public:
    IsometricRenderHelper(Tiled::MapRenderer *renderer,
                          QSGNode *parent,
                          const MapItem *mapItem,
                          const TileLayer *layer,
                          const QRect &visibleScreenRect) :
        mRenderer(renderer),
        mParent(parent),
        mLayer(layer),
        mVisibleScreenRect(visibleScreenRect),
        mTilesetHelper(mapItem),
        mTileWidth(mapItem->map()->tileWidth()),
        mTileHeight(mapItem->map()->tileHeight())
    {
    }

    void addTilesToNode();

private:
    void appendTileData(const Cell &cell, const QPointF &screenPos);

    Tiled::MapRenderer *mRenderer;
    QSGNode *mParent;
    const TileLayer *mLayer;
    const QRect mVisibleScreenRect;
    TilesetHelper mTilesetHelper;
    const int mTileWidth;
    const int mTileHeight;
    QVector<TileData> mTileData;
};

void IsometricRenderHelper::appendTileData(const Cell &cell, const QPointF &screenPos)
{
    if (cell.isEmpty())
        return;

    Tileset *tileset = cell.tileset();

    if (tileset != mTilesetHelper.tileset() || mTileData.size() == TilesNode::MaxTileCount) {
        if (!mTileData.isEmpty()) {
            mParent->appendChildNode(new TilesNode(mTilesetHelper.texture(),
                                                   mTileData));
            mTileData.resize(0);
        }

        mTilesetHelper.setTileset(tileset);
    }

    if (!mTilesetHelper.texture())
        return;

    Tile *tile = cell.tile();
    if (!tile) {
        // todo: render "missing tile" marker
        return;
    }

    TileData data;
    data.x = screenPos.x() - mTileWidth / 2;
    data.y = screenPos.y() - mTileHeight / 2;
    const QSize size = cell.tile()->size();
    data.width = size.width();
    data.height = size.height();
    data.flippedHorizontally = cell.flippedHorizontally();
    data.flippedVertically = cell.flippedVertically();
    mTilesetHelper.setTextureCoordinates(data, cell);
    mTileData.append(data);
}

// Mostly copied from IsometricRenderer::drawTileLayer().
void IsometricRenderHelper::addTilesToNode()
{
    if (mVisibleScreenRect.isEmpty())
        return;

    auto tileRenderFunction = [=](const Cell &cell, const QPointF &pos, const QSizeF &/*size*/) {
        appendTileData(cell, pos);
    };
    mRenderer->drawTileLayer(mLayer, tileRenderFunction, mVisibleScreenRect);

    if (!mTileData.isEmpty())
        mParent->appendChildNode(new TilesNode(mTilesetHelper.texture(), mTileData));
}

} // anonymous namespace


TileLayerItem::TileLayerItem(TileLayer *layer, MapRenderer *renderer,
                             MapItem *parent)
    : QQuickItem(parent)
    , mLayer(layer)
    , mRenderer(renderer)
    , mVisibleTiles(parent->visibleTileArea(layer))
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

    const MapItem *mapItem = static_cast<MapItem*>(parentItem());
    if (mLayer->map()->orientation() == Map::Orthogonal)
        drawOrthogonalTileLayer(node, mapItem, mLayer, mVisibleTiles);
    else
        IsometricRenderHelper(mRenderer, node, mapItem, mLayer, mapItem->visibleArea().toRect()).addTilesToNode();

    return node;
}

void TileLayerItem::updateVisibleTiles()
{
    const MapItem *mapItem = static_cast<MapItem*>(parentItem());
    const QRect rect = mapItem->visibleTileArea(mLayer);

    // For isometric rendering, we need to use the visible screen rect,
    // so the visibleTileArea optimisation doesn't work for us.
    // If we don't do this, some tiles on the edge of the screen won't be rendered.
    const bool shouldUpdate = mapItem->map()->orientation() == Map::Isometric || mVisibleTiles != rect;
    mVisibleTiles = rect;
    if (shouldUpdate)
        update();
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
    setZ(position.y() * parent->map()->tileHeight());
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
