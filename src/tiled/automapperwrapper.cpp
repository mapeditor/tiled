/*
 * automapperwrapper.cpp
 * Copyright 2010-2011, Stefan Beller, stefanbeller@googlemail.com
 * Copyright 2018-2021, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

AutoMapperWrapper::AutoMapperWrapper(MapDocument *mapDocument,
                                     const std::vector<std::unique_ptr<AutoMapper> > &autoMappers,
                                     const QRegion &where,
                                     const TileLayer *touchedLayer)
    : mMapDocument(mapDocument)
{
    for (const auto &autoMapper : autoMappers) {
        autoMapper->prepareAutoMap();

        // Store a copy of each output tile layer before AutoMapping.
        for (TileLayer *layer : autoMapper->outputTileLayers()) {
            if (mOutputTileLayers.find(layer) != mOutputTileLayers.end())
                continue;

            OutputLayerData &data = mOutputTileLayers[layer];
            data.before.reset(layer->clone());
        }
    }

    // During "AutoMap while drawing", keep track of the touched layers, so we
    // can skip any rule maps that don't have these layers as input entirely.
    QList<const TileLayer*> touchedTileLayers;
    QList<const TileLayer*> *touchedTileLayersPtr = nullptr;
    if (touchedLayer) {
        touchedTileLayers.append(touchedLayer);
        touchedTileLayersPtr = &touchedTileLayers;
    }

    // use a copy of the region, so each AutoMapper can manipulate it and the
    // following AutoMappers do see the impact
    QRegion region(where);
    QRegion appliedRegion;
    QRegion *appliedRegionPtr = &appliedRegion;
    const Map *map = mapDocument->map();
    const QRegion mapRect(0, 0, map->width(), map->height());

    for (const auto &autoMapper : autoMappers) {
        // stop expanding region when it's already the entire fixed-size map
        if (appliedRegionPtr && (!map->infinite() && (mapRect - region).isEmpty()))
            appliedRegionPtr = nullptr;

        if (!touchedTileLayers.isEmpty()) {
            if (std::none_of(touchedTileLayers.cbegin(),
                             touchedTileLayers.cend(),
                             [&] (const TileLayer *tileLayer) { return autoMapper->ruleLayerNameUsed(tileLayer->name()); }))
                continue;
        }

        autoMapper->autoMap(region, touchedTileLayersPtr, appliedRegionPtr);

        if (appliedRegionPtr) {
            // expand where with modified area
            region |= std::exchange(appliedRegion, QRegion());

            if (!map->infinite())       // but keep within map boundaries
                region &= mapRect;
        }
    }

    for (std::pair<TileLayer* const, OutputLayerData> &pair : mOutputTileLayers) {
        auto target = pair.first;
        auto &before = pair.second.before;

        MapDocument::TileLayerChangeFlags flags;

        if (before->drawMargins() != target->drawMargins())
            flags |= MapDocument::LayerDrawMarginsChanged;
        if (before->bounds() != target->bounds())
            flags |= MapDocument::LayerBoundsChanged;

        if (flags)
            emit mMapDocument->tileLayerChanged(target, flags);

        // reduce memory usage by saving only diffs
        pair.second.region = before->computeDiffRegion(target);
        const QRect diffRect = pair.second.region.boundingRect();

        auto beforeDiff = before->copy(pair.second.region);
        beforeDiff->setPosition(diffRect.topLeft());
        beforeDiff->setName(before->name());

        auto afterDiff = target->copy(pair.second.region);
        afterDiff->setPosition(diffRect.topLeft());
        afterDiff->setName(target->name());

        pair.second.before = std::move(beforeDiff);
        pair.second.after = std::move(afterDiff);
    }

    for (const auto &autoMapper : autoMappers)
        autoMapper->finalizeAutoMap();
}

AutoMapperWrapper::~AutoMapperWrapper()
{
}

void AutoMapperWrapper::undo()
{
    for (std::pair<TileLayer* const, OutputLayerData> &pair : mOutputTileLayers)
        patchLayer(pair.first, *pair.second.before, pair.second.region);
}

void AutoMapperWrapper::redo()
{
    for (std::pair<TileLayer* const, OutputLayerData> &pair : mOutputTileLayers)
        patchLayer(pair.first, *pair.second.after, pair.second.region);
}

void AutoMapperWrapper::patchLayer(TileLayer *target, const TileLayer &layer, const QRegion &region)
{
    target->setCells(layer.x() - target->x(),
                     layer.y() - target->y(),
                     &layer,
                     region.translated(-target->position()));

    emit mMapDocument->regionChanged(region, target);
}
