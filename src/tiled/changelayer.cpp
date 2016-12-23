/*
 * changelayer.cpp
 * Copyright 2012-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changelayer.h"

#include "tilelayer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"

#include <math.h>
#include <QCoreApplication>

namespace Tiled {
namespace Internal {

SetLayerVisible::SetLayerVisible(MapDocument *mapDocument,
                                 int layerIndex,
                                 bool visible)
    : mMapDocument(mapDocument)
    , mLayerIndex(layerIndex)
    , mVisible(visible)
{
    if (visible)
        setText(QCoreApplication::translate("Undo Commands",
                                            "Show Layer"));
    else
        setText(QCoreApplication::translate("Undo Commands",
                                            "Hide Layer"));
}

void SetLayerVisible::swap()
{
    const Layer *layer = mMapDocument->map()->layerAt(mLayerIndex);
    const bool previousVisible = layer->isVisible();
    mMapDocument->layerModel()->setLayerVisible(mLayerIndex, mVisible);
    mVisible = previousVisible;
}


SetLayerOpacity::SetLayerOpacity(MapDocument *mapDocument,
                                 int layerIndex,
                                 float opacity)
    : mMapDocument(mapDocument)
    , mLayerIndex(layerIndex)
    , mOldOpacity(mMapDocument->map()->layerAt(layerIndex)->opacity())
    , mNewOpacity(opacity)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Opacity"));
}

bool SetLayerOpacity::mergeWith(const QUndoCommand *other)
{
    const SetLayerOpacity *o = static_cast<const SetLayerOpacity*>(other);
    if (!(mMapDocument == o->mMapDocument &&
          mLayerIndex == o->mLayerIndex))
        return false;

    mNewOpacity = o->mNewOpacity;
    return true;
}

void SetLayerOpacity::setOpacity(float opacity)
{
    mMapDocument->layerModel()->setLayerOpacity(mLayerIndex, opacity);
}


SetLayerOffset::SetLayerOffset(MapDocument *mapDocument,
                               int layerIndex,
                               const QPointF &offset)
    : mMapDocument(mapDocument)
    , mLayerIndex(layerIndex)
    , mOldOffset(mMapDocument->map()->layerAt(layerIndex)->offset())
    , mNewOffset(offset)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Offset"));
}

void SetLayerOffset::setOffset(const QPointF &offset)
{
    mMapDocument->layerModel()->setLayerOffset(mLayerIndex, offset);
}

SetLayerTileSize::SetLayerTileSize(MapDocument *mapDocument,
                                   int index,
                                   const QSize &tileSize)
    : mMapDocument(mapDocument)
    , mIndex(index)
    , mOriginalLayer(nullptr)
{
    Q_ASSERT(mIndex != -1);

    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer TileSize"));

    Map* map = mMapDocument->map();
    mResizedLayer = map->layerAt(index)->clone();
    mResizedLayer->setTileSize(tileSize);

    const QSize newSize = {
        (int)round(((float)map->width()*(float)map->tileWidth())/(float)tileSize.width()),
        (int)round(((float)map->height()*(float)map->tileHeight())/(float)tileSize.height())
    };

    if (mResizedLayer->asTileLayer()) {
        mResizedLayer->asTileLayer()->resize(newSize, QPoint(0, 0));
    } else {
        mResizedLayer->setSize(newSize);
    }
}

SetLayerTileSize::~SetLayerTileSize()
{
    delete mResizedLayer;
    delete mOriginalLayer;
}

void SetLayerTileSize::undo()
{
    Q_ASSERT(!mResizedLayer);
    mResizedLayer = swapLayer(mOriginalLayer);
    mOriginalLayer = nullptr;
}

void SetLayerTileSize::redo()
{
    Q_ASSERT(!mOriginalLayer);
    mOriginalLayer = swapLayer(mResizedLayer);
    mResizedLayer = nullptr;
}

Layer *SetLayerTileSize::swapLayer(Layer *layer)
{
    const int currentIndex = mMapDocument->currentLayerIndex();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(mIndex);
    layerModel->insertLayer(mIndex, layer);

    if (mIndex == currentIndex)
        mMapDocument->setCurrentLayerIndex(mIndex);

    return replaced;
}


} // namespace Internal
} // namespace Tiled
