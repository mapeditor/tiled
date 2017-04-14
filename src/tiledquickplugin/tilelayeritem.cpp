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
        texture = window->createTextureFromImage(QImage(tileset->imageSource()));
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
        : mMapItem(mapItem)
        , mWindow(mapItem->window())
        , mTileset(0)
        , mTexture(0)
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
        const int tileId = cell.tile->id();
        const int column = tileId % mTilesPerRow;
        const int row = tileId / mTilesPerRow;

        data.tx = column * mTileHSpace + mMargin;
        data.ty = row * mTileVSpace + mMargin;
    }

private:
    const MapItem *mMapItem;
    QQuickWindow *mWindow;
    Tileset *mTileset;
    QSGTexture *mTexture;
    int mMargin;
    int mTileHSpace;
    int mTileVSpace;
    int mTilesPerRow;
};

/**
 * Draws an orthogonal tile layer by adding nodes to the scene graph. As long
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

    for (int y = rect.top(); y <= rect.bottom(); ++y) {
        for (int x = rect.left(); x <= rect.right(); ++x) {
            const Cell &cell = layer->cellAt(x, y);
            if (cell.isEmpty())
                continue;

            Tileset *tileset = cell.tile->tileset();

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

            const QSize size = cell.tile->size();
            const QPoint offset = tileset->tileOffset();

            TileData data;
            data.x = x * tileWidth + offset.x();
            data.y = (y + 1) * tileHeight - tileset->tileHeight() + offset.y();
            data.width = size.width();
            data.height = size.height();
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
    IsometricRenderHelper(QSGNode *parent,
                          const MapItem *mapItem,
                          const TileLayer *layer,
                          const QRect &rect) :
        mParent(parent),
        mMapItem(mapItem),
        mLayer(layer),
        mRect(rect),
        mTilesetHelper(mapItem),
        mTilesWide(rect.width()),
        mTilesHigh(rect.height()),
        mTileWidth(mapItem->map()->tileWidth()),
        mTileHeight(mapItem->map()->tileHeight()),
        mTilesCreated(0)
    {
    }

    void addTilesToNode();

private:
    QPoint indexToMapPos(int index) const;
    QPoint mapToScreen(const QPoint &mapPosInTiles) const;
    void appendTileData(int index);

    QSGNode *mParent;
    const MapItem *mMapItem;
    const TileLayer *mLayer;
    const QRect mRect;
    TilesetHelper mTilesetHelper;
    const int mTilesWide;
    const int mTilesHigh;
    const int mTileWidth;
    const int mTileHeight;
    QVector<TileData> mTileData;
    int mTilesCreated;
};

QPoint IsometricRenderHelper::indexToMapPos(int index) const
{
    return QPoint((index % mTilesWide), qFloor(index / mTilesWide));
}

// http://clintbellanger.net/articles/isometric_math/
QPoint IsometricRenderHelper::mapToScreen(const QPoint &mapPosInTiles) const
{
    return QPoint((mapPosInTiles.x() - mapPosInTiles.y()) * (mTileWidth / 2),
                  (mapPosInTiles.x() + mapPosInTiles.y()) * (mTileHeight / 2));
}

void IsometricRenderHelper::appendTileData(int index)
{
    const QPoint mapPos = indexToMapPos(index);
    const Cell &cell = mLayer->cellAt(mapPos.x(), mapPos.y());
    if (cell.isEmpty())
        return;

    Tileset *tileset = cell.tile->tileset();

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

    const QPoint screenPos = mapToScreen(mapPos);
//    qDebug() << "appendTileData:" << "index" << index << mTilesWide << "*" << mTilesHigh << "(" << mTilesWide * mTilesHigh << ")" << screenPos;
    TileData data;
    data.x = screenPos.x();
    data.y = screenPos.y();
    const QSize size = cell.tile->size();
    data.width = size.width();
    data.height = size.height();
    mTilesetHelper.setTextureCoordinates(data, cell);
    mTileData.append(data);
    ++mTilesCreated;
}

// TODO: make this function work with a subset of the entire layer rect
void IsometricRenderHelper::addTilesToNode()
{
    mTileData.reserve(TilesNode::MaxTileCount);

    const int tileCount = mTilesWide * mTilesHigh;
    if (tileCount == 0)
        return;

    int i = 0;
    appendTileData(i);

    int z = 1;
    // This loop takes us down to the widest point of the "diamond".
    // For example, with a 15 x 10 map, this loop will create items
    // up to and including the row starting with index 135 (marked (a)):
    //
    //                  0
    //               15    1
    //            30   16     2
    //         ..    ..   ..    ..
    //      135        ...         9                   (a)
    //         136 122       ...  24 10
    //            137 123    ...     25 11
    //              ..       ...           ..
    //                140 126  ...       28  14        (b)
    //                   ..     ...       ..
    //                      ..   ...   ..
    //                         148  134
    //                            149
    //
    // The process is similar for a differently sized map;
    // we swap some checks around to check for height instead of width, for example.
    // Start off assuming that one is bigger than the other to save some code.
    int smallerDimension = mTilesHigh;
    int largerDimension = mTilesWide;
    bool widthLarger = true;
    // Check that our assumption is true.
    if (mTilesWide < mTilesHigh) {
        smallerDimension = mTilesWide;
        largerDimension = mTilesHigh;
        widthLarger = false;
    }

    while (z < smallerDimension) {
        // * mTilesWide because we always start from the left
        // and move to the right.
        i = z * mTilesWide;

        do {
            appendTileData(i);
            i -= mTilesWide - 1;
        } while (i > 0);

        ++z;
    }

    int rowEndIndex = widthLarger ? 0 : (mTilesWide * 2) - 1;
    while (z < largerDimension) {
        // We've reached the "widest" point of the "diamond"
        // ( (a) on the diagram above).
        // From now on we'll be completing the next part.
        // If the map is square, this loop will be skipped.
        // However, for the 15 x 10 example we've been using,
        // The loop will continue until we've created the last row
        // of the "widest" part ( (b) on the diagram above).

        if (mTilesWide > mTilesHigh) {
            // z will be 10 the first time this code is hit, so:
            //         z - (mTilesHigh - 1)
            //        10 - (    10    - 1)
            //        10 - (          9  )
            //           1
            // The first part of the calculation gets us to the first index
            // of the last "row" we were on:
            //     (mTilesHigh - 1) * mTilesWide
            //     (10        - 1) *     15
            //                9    *     15
            //                    135
            // 135 + 1 = 136. The index then increases by 1 from then on.
            i = ((mTilesHigh - 1) * mTilesWide) + (z - (mTilesHigh - 1));
        } else {
            i = (mTilesWide * z);
        }

        do {
            appendTileData(i);
            i -= mTilesWide - 1;
        } while (widthLarger ? i > 0 : i >= rowEndIndex);

        ++z;
        rowEndIndex += mTilesWide;
    }

    // Go back to the index of the last tile that we created.
    i += mTilesWide - 1;

    if (widthLarger) {
        rowEndIndex = (mTilesWide * 2) - 1;
    }
    while (mTilesCreated < tileCount) {
        i = ((mTilesHigh - 1) * mTilesWide) + (z - (mTilesHigh - 1));

        do {
            appendTileData(i);
            i -= mTilesWide - 1;
        } while (i >= rowEndIndex);

        ++z;
        rowEndIndex += mTilesWide;
    }

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

    connect(parent, SIGNAL(visibleAreaChanged()), SLOT(updateVisibleTiles()));

    syncWithTileLayer();
    setOpacity(mLayer->opacity());
}

void TileLayerItem::syncWithTileLayer()
{
    const QRectF boundingRect = mRenderer->boundingRect(mLayer->bounds());
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
        IsometricRenderHelper(node, mapItem, mLayer, mVisibleTiles).addTilesToNode();

    return node;
}

void TileLayerItem::updateVisibleTiles()
{
    const MapItem *mapItem = static_cast<MapItem*>(parentItem());
    const QRect rect = mLayer->map()->orientation() == Map::Orthogonal
            ? mapItem->visibleTileArea(mLayer) : mLayer->bounds();

    if (mVisibleTiles != rect) {
        mVisibleTiles = rect;
        update();
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
        Tileset *tileset = mCell.tile->tileset();
        helper.setTileset(tileset);

        if (!helper.texture())
            return 0;

        const Map *map = mapItem->map();
        const int tileWidth = map->tileWidth();
        const int tileHeight = map->tileHeight();

        const QSize size = mCell.tile->size();
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
