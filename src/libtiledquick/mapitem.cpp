/*
 * mapitem.cpp
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

#include "mapitem.h"

#include "tilelayeritem.h"
#include "tilesethelper.h"

#include "map.h"
#include "maprenderer.h"

using namespace TiledQuick;

MapItem::MapItem(QQuickItem *parent)
    : QQuickItem(parent)
    , mNewObjectsPreviewItem(nullptr, nullptr, this)
{
}

MapItem::~MapItem()
{
    TilesetHelper::deleteInstance();
};

void MapItem::setMap(Tiled::EditableMap *editableMap)
{
    if (mEditableMap == editableMap)
        return;

    if (mEditableMap && mEditableMap->mapDocument()) {
        Tiled::MapDocument *oldMapDocument = mEditableMap->mapDocument();
        disconnect(oldMapDocument, &Tiled::MapDocument::regionChanged, this, &MapItem::repaintRegion);
        disconnect(oldMapDocument, &Tiled::MapDocument::mapObjectsChanged, this, &MapItem::repaintObjects);
        disconnect(oldMapDocument, &Tiled::MapDocument::mapResized, this, &MapItem::refresh);
    }

    if (editableMap && editableMap->mapDocument()) {
        Tiled::MapDocument *mapDocument = editableMap->mapDocument();
        connect(mapDocument, &Tiled::MapDocument::regionChanged, this, &MapItem::repaintRegion);
        connect(mapDocument, &Tiled::MapDocument::mapObjectsChanged, this, &MapItem::repaintObjects);
        connect(mapDocument, &Tiled::MapDocument::mapResized, this, &MapItem::refresh);
    }

    mEditableMap = editableMap;
    mMap = editableMap ? editableMap->map() : nullptr;

    refresh();
    emit mapChanged();
}

void MapItem::setVisibleArea(const QRectF &visibleArea)
{
    mVisibleArea = visibleArea;
    emit visibleAreaChanged();
}

void MapItem::setZoom(const qreal &zoom)
{
    if (mZoom == zoom)
        return;

    mZoom = zoom;

    for (auto objectGroup : std::as_const(mObjectGroupItems))
        objectGroup->setZoom(zoom);
    mNewObjectsPreviewItem.setZoom(zoom);

    // Since the tool brush is always fully rendered and does not
    // receive the visibleAreaChanged signal upon zoom changes,
    // we need to update it here.
    mNewObjectsPreviewItem.update();

    emit zoomChanged();
}

void MapItem::setNewObjectsPreview(Tiled::ObjectGroup *objects)
{
    if (mNewObjectsPreviewItem.group() == objects)
        return;

    mNewObjectsPreviewItem.setObjectGroup(objects);
    emit newObjectsPreviewChanged();
}

void MapItem::setMousePos(const QPointF &pos)
{
    if (mNewObjectsPreviewItem.mousePos() == pos)
        return;

    mNewObjectsPreviewItem.setMousePos(pos);
    emit mousePosChanged();
}

void MapItem::repaintPreview()
{
    mNewObjectsPreviewItem.update();
}

QRectF MapItem::boundingRect() const
{
    if (!mRenderer)
        return QRectF();

    return mRenderer->mapBoundingRect();
}

QSize MapItem::tileSize() const
{
    if (!mMap)
        return {0,0};

    return mMap->tileSize();
}

QPointF MapItem::screenToTileCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->screenToTileCoords(x, y);
}

QPointF MapItem::screenToTileCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->screenToTileCoords(position);
}

QPointF MapItem::tileToScreenCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->tileToScreenCoords(x, y);
}

QPointF MapItem::tileToScreenCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->tileToScreenCoords(position);
}

QPointF MapItem::screenToPixelCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->screenToPixelCoords(x, y);
}

QPointF MapItem::screenToPixelCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->screenToPixelCoords(position);
}

QPointF MapItem::pixelToScreenCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->pixelToScreenCoords(x, y);
}

QPointF MapItem::pixelToScreenCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->pixelToScreenCoords(position);
}

QPointF MapItem::pixelToTileCoords(qreal x, qreal y) const
{
    if (!mRenderer)
        return QPointF(x, y);
    return mRenderer->pixelToTileCoords(x, y);
}

QPointF MapItem::pixelToTileCoords(const QPointF &position) const
{
    if (!mRenderer)
        return position;
    return mRenderer->pixelToTileCoords(position);
}

void MapItem::componentComplete()
{
    QQuickItem::componentComplete();
    if (mMap)
        refresh();
}

void MapItem::refresh()
{
    if (!isComponentComplete())
        return;

    qDeleteAll(mTileLayerItems);
    mTileLayerItems.clear();

    qDeleteAll(mObjectGroupItems);
    mObjectGroupItems.clear();

    mRenderer = nullptr;

    if (!mMap)
        return;

    mRenderer = Tiled::MapRenderer::create(mMap);
    mNewObjectsPreviewItem.setRenderer(mRenderer.get());
    mNewObjectsPreviewItem.setZoom(mZoom);

    for (Tiled::Layer *layer : mMap->layers()) {
        if (Tiled::TileLayer *tl = layer->asTileLayer()) {
            TileLayerItem *layerItem = new TileLayerItem(tl, mRenderer.get(), this);
            mTileLayerItems.append(layerItem);
        }

        if (Tiled::ObjectGroup *og = layer->asObjectGroup()) {
            ObjectGroupItem *groupItem = new ObjectGroupItem(og, mRenderer.get(), this);
            mObjectGroupItems.append(groupItem);
            groupItem->setZoom(mZoom);
        }
    }

    const QRect rect = mRenderer->mapBoundingRect();
    setImplicitSize(rect.width(), rect.height());
}

void MapItem::repaintRegion(const QRegion &, Tiled::TileLayer *tileLayer)
{
    for (TileLayerItem *tileLayerItem : std::as_const(mTileLayerItems)) {
        if (tileLayer == tileLayerItem->layer()) {
            // TODO: Update only the region edited
            tileLayerItem->update();
            break;
        }
    }
}

void MapItem::repaintObjects(const QList<Tiled::MapObject*> &objects, ObjectGroup *group)
{
    QSet<Tiled::ObjectGroup*> changedGroups;
    for (auto object : objects)
        changedGroups.insert(object->objectGroup());

    if (changedGroups.size() == 0 && !group)
        return;

    // TODO: Update only nodes containing affected objects rather than entire layers
    for (auto objectGroupItem : std::as_const(mObjectGroupItems)) {
        if (objectGroupItem->group() == group || changedGroups.contains(objectGroupItem->group()))
        {
            // objectGroupItem->syncWithObjectGroup();
            objectGroupItem->update();
        }
    }

    emit mapObjectsChanged();
}
