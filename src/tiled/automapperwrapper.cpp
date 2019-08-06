/*
 * automapperwrapper.cpp
 * Copyright 2010-2011, Stefan Beller, stefanbeller@googlemail.com
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

#include "automapperwrapper.h"

#include "map.h"
#include "mapdocument.h"
#include "tile.h"
#include "tilelayer.h"

#include "qtcompat_p.h"

using namespace Tiled;

AutoMapperWrapper::AutoMapperWrapper(MapDocument *mapDocument,
                                     QVector<AutoMapper*> autoMappers,
                                     QRegion *where)
{
    mMapDocument = mapDocument;
    Map *map = mMapDocument->map();

    QSet<QString> touchedTileLayers;
    int index = 0;
    while (index < autoMappers.size()) {
        AutoMapper *a = autoMappers.at(index);
        if (a->prepareAutoMap()) {
            touchedTileLayers |= a->touchedTileLayers();
            index++;
        } else {
            autoMappers.remove(index);
        }
    }
    for (const QString &layerName : qAsConst(touchedTileLayers)) {
        const int layerIndex = map->indexOfLayer(layerName, Layer::TileLayerType);
        Q_ASSERT(layerIndex != -1);
        mLayersBefore.append(static_cast<TileLayer*>(map->layerAt(layerIndex)->clone()));
    }

    for (AutoMapper *a : autoMappers)
        a->autoMap(where);

    int beforeIndex = 0;
    for (const QString &layerName : qAsConst(touchedTileLayers)) {
        const int layerIndex = map->indexOfLayer(layerName, Layer::TileLayerType);
        // layer index exists, because AutoMapper is still alive, don't check
        Q_ASSERT(layerIndex != -1);
        TileLayer *before = mLayersBefore.at(beforeIndex);
        TileLayer *after = static_cast<TileLayer*>(map->layerAt(layerIndex));

        MapDocument::TileLayerChangeFlags flags;

        if (before->drawMargins() != after->drawMargins())
            flags |= MapDocument::LayerDrawMarginsChanged;
        if (before->bounds() != after->bounds())
            flags |= MapDocument::LayerBoundsChanged;

        if (flags)
            emit mMapDocument->tileLayerChanged(after, flags);

        // reduce memory usage by saving only diffs
        QRect diffRegion = before->computeDiffRegion(after).boundingRect();
        TileLayer *before1 = before->copy(diffRegion);
        TileLayer *after1 = after->copy(diffRegion);

        before1->setPosition(diffRegion.topLeft());
        after1->setPosition(diffRegion.topLeft());
        before1->setName(before->name());
        after1->setName(after->name());
        mLayersBefore.replace(beforeIndex, before1);
        mLayersAfter.append(after1);

        delete before;
        ++beforeIndex;
    }

    for (AutoMapper *a : autoMappers)
        a->cleanAll();
}

AutoMapperWrapper::~AutoMapperWrapper()
{
    qDeleteAll(mLayersAfter);
    qDeleteAll(mLayersBefore);
}

void AutoMapperWrapper::undo()
{
    Map *map = mMapDocument->map();
    for (TileLayer *layer : qAsConst(mLayersBefore)) {
        const int layerIndex = map->indexOfLayer(layer->name(), Layer::TileLayerType);
        if (layerIndex != -1)
            patchLayer(layerIndex, layer);
    }
}

void AutoMapperWrapper::redo()
{
    Map *map = mMapDocument->map();
    for (TileLayer *layer : qAsConst(mLayersAfter)) {
        const int layerIndex = map->indexOfLayer(layer->name(), Layer::TileLayerType);
        if (layerIndex != -1)
            patchLayer(layerIndex, layer);
    }
}

void AutoMapperWrapper::patchLayer(int layerIndex, TileLayer *layer)
{
    Map *map = mMapDocument->map();
    QRect b = layer->rect();

    Q_ASSERT(map->layerAt(layerIndex)->asTileLayer());
    TileLayer *t = static_cast<TileLayer*>(map->layerAt(layerIndex));

    t->setCells(b.left() - t->x(),
                b.top() - t->y(),
                layer,
                b.translated(-t->position()));
    emit mMapDocument->regionChanged(b, t);
}
