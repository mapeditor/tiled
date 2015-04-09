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

using namespace Tiled;
using namespace Tiled::Internal;

AutoMapperWrapper::AutoMapperWrapper(MapDocument *mapDocument,
                                     QVector<AutoMapper*> autoMapper,
                                     QRegion *where)
{
    mMapDocument = mapDocument;
    Map *map = mMapDocument->map();

    QSet<QString> touchedLayers;
    int index = 0;
    while (index < autoMapper.size()) {
        AutoMapper *a = autoMapper.at(index);
        if (a->prepareAutoMap()) {
            touchedLayers|= a->getTouchedTileLayers();
            index++;
        } else {
            autoMapper.remove(index);
        }
    }
    foreach (const QString &layerName, touchedLayers) {
        const int layerindex = map->indexOfLayer(layerName);
        Q_ASSERT(layerindex != -1);
        mLayersBefore << static_cast<TileLayer*>(map->layerAt(layerindex)->clone());
    }

    foreach (AutoMapper *a, autoMapper)
        a->autoMap(where);

    foreach (const QString &layerName, touchedLayers) {
        const int layerindex = map->indexOfLayer(layerName);
        // layerindex exists, because AutoMapper is still alive, dont check
        Q_ASSERT(layerindex != -1);
        mLayersAfter << static_cast<TileLayer*>(map->layerAt(layerindex)->clone());
    }
    // reduce memory usage by saving only diffs
    Q_ASSERT(mLayersAfter.size() == mLayersBefore.size());
    for (int i = 0; i < mLayersAfter.size(); ++i) {
        TileLayer *before = mLayersBefore.at(i);
        TileLayer *after = mLayersAfter.at(i);
        QRect diffRegion = before->computeDiffRegion(after).boundingRect();

        TileLayer *before1 = before->copy(diffRegion);
        TileLayer *after1 = after->copy(diffRegion);

        before1->setPosition(diffRegion.topLeft());
        after1->setPosition(diffRegion.topLeft());
        before1->setName(before->name());
        after1->setName(after->name());
        mLayersBefore.replace(i, before1);
        mLayersAfter.replace(i, after1);

        delete before;
        delete after;
    }

    foreach (AutoMapper *a, autoMapper) {
        a->cleanAll();
    }
}

AutoMapperWrapper::~AutoMapperWrapper()
{
    QVector<TileLayer*>::iterator i;
    for (i = mLayersAfter.begin(); i != mLayersAfter.end(); ++i)
        delete *i;
    for (i = mLayersBefore.begin(); i != mLayersBefore.end(); ++i)
        delete *i;
}

void AutoMapperWrapper::undo()
{
    Map *map = mMapDocument->map();
    QVector<TileLayer*>::iterator i;
    for (i = mLayersBefore.begin(); i != mLayersBefore.end(); ++i) {
        const int layerindex = map->indexOfLayer((*i)->name());
        if (layerindex != -1)
            patchLayer(layerindex, *i);
    }
}

void AutoMapperWrapper::redo()
{
    Map *map = mMapDocument->map();
    QVector<TileLayer*>::iterator i;
    for (i = mLayersAfter.begin(); i != mLayersAfter.end(); ++i) {
        const int layerindex = (map->indexOfLayer((*i)->name()));
        if (layerindex != -1)
            patchLayer(layerindex, *i);
    }

}

void AutoMapperWrapper::patchLayer(int layerIndex, TileLayer *layer)
{
    Map *map = mMapDocument->map();
    QRect b = layer->bounds();

    Q_ASSERT(map->layerAt(layerIndex)->asTileLayer());
    TileLayer *t = static_cast<TileLayer*>(map->layerAt(layerIndex));

    t->setCells(b.left() - t->x(), b.top() - t->y(), layer,
                b.translated(-t->position()));
    mMapDocument->emitRegionChanged(b);
}
