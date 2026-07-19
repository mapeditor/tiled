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

#include "map.h"
#include "maprenderer.h"

#include "tilesethelper.h"
#include "tilesnode.h"

#include <QtMath>
#include <QQuickWindow>

using namespace Tiled;
using namespace TiledQuick;

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

    TilesetHelper helper = TilesetHelper::instance(static_cast<MapItem*>(parentItem()));

    QVector<TileData> tileData;
    tileData.reserve(TilesNode::MaxTileCount);

    /**
     * Draws the tiles by adding nodes to the scene graph. When sequentially
     * drawn tiles are using the same tileset, they will share a single
     * geometry node.
     */
    auto tileRenderFunction = [&](QPoint tilePos, const QPointF &screenPos) {
        const Cell &cell = mLayer->cellAt(tilePos);
        Tileset *tileset = cell.tileset();
        if (!tileset)
            return;

        if (tileset != helper.tileset() || tileData.size() == TilesNode::MaxTileCount) {
            if (!tileData.isEmpty()) {
                node->appendChildNode(new TilesNode(helper.texture(), tileData));
                tileData.clear();
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
        const auto tile = tileset->findTile(cell.tileId());
        const QSize size = (tile && !tile->image().isNull()) ? tile->size() : mRenderer->map()->tileSize();

        TileData data;
        data.x = static_cast<float>(screenPos.x()) + offset.x();
        data.y = static_cast<float>(screenPos.y() - size.height()) + offset.y();
        data.width = static_cast<float>(size.width());
        data.height = static_cast<float>(size.height());
        data.flippedHorizontally = cell.flippedHorizontally();
        data.flippedVertically = cell.flippedVertically();
        helper.setTextureCoordinates(data.tx, data.ty, cell);
        tileData.append(data);
    };

    mRenderer->drawTileLayer(tileRenderFunction, mVisibleArea);

    if (!tileData.isEmpty())
        node->appendChildNode(new TilesNode(helper.texture(), tileData));

    return node;
}

void TileLayerItem::updateVisibleTiles()
{
    const MapItem *mapItem = static_cast<MapItem*>(parentItem());

    QRectF rect = mapItem->visibleArea();

    QMargins drawMargins = mLayer->drawMargins();
    drawMargins.setTop(drawMargins.top() - mRenderer->map()->tileHeight());
    drawMargins.setRight(drawMargins.right() - mRenderer->map()->tileWidth());

    rect.adjust(-drawMargins.right(),
                -drawMargins.bottom(),
                drawMargins.left(),
                drawMargins.top());

    rect &= mRenderer->boundingRect(mLayer->localBounds());

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

        const QSize tileSize = mapItem->map()->map()->tileSize();
        const QSize size = tile->size();
        const QPoint offset = tileset->tileOffset();

        QVector<TileData> data(1);
        data[0].x = mPosition.x() * tileSize.width() + offset.x();
        data[0].y = (mPosition.y() + 1) * tileSize.height() - tileset->tileHeight() + offset.y();
        data[0].width = size.width();
        data[0].height = size.height();
        helper.setTextureCoordinates(data[0].tx, data[0].ty, mCell);

        node = new TilesNode(helper.texture(), data);
    }

    return node;
}
