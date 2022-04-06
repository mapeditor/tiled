/*
 * automapperwrapper.cpp
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2018-2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "addremovelayer.h"
#include "addremovemapobject.h"
#include "addremovetileset.h"
#include "changeproperties.h"
#include "containerhelpers.h"
#include "map.h"
#include "mapdocument.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilepainter.h"

using namespace Tiled;

AutoMapperWrapper::AutoMapperWrapper(MapDocument *mapDocument,
                                     const QVector<AutoMapper *> &autoMappers,
                                     const QRegion &where,
                                     const TileLayer *touchedLayer)
    : mMapDocument(mapDocument)
{
    AutoMappingContext context(mapDocument);

    for (const auto autoMapper : autoMappers)
        autoMapper->prepareAutoMap(context);

    // During "AutoMap while drawing", keep track of the touched layers, so we
    // can skip any rule maps that doesn't have these layers as input entirely.
    if (touchedLayer)
        context.touchedTileLayers.append(touchedLayer);

    // use a copy of the region, so each AutoMapper can manipulate it and the
    // following AutoMappers do see the impact
    QRegion region(where);
    QRegion appliedRegion;
    QRegion *appliedRegionPtr = &appliedRegion;
    const Map *map = mapDocument->map();
    const QRegion mapRect(0, 0, map->width(), map->height());

    for (const auto autoMapper : autoMappers) {
        // stop expanding region when it's already the entire fixed-size map
        if (appliedRegionPtr && (!map->infinite() && (mapRect - region).isEmpty()))
            appliedRegionPtr = nullptr;

        if (!context.touchedTileLayers.isEmpty()) {
            if (std::none_of(context.touchedTileLayers.cbegin(),
                             context.touchedTileLayers.cend(),
                             [&] (const TileLayer *tileLayer) { return autoMapper->ruleLayerNameUsed(tileLayer->name()); }))
                continue;
        }

        autoMapper->autoMap(region, appliedRegionPtr, context);

        if (appliedRegionPtr) {
            // expand where with modified area
            region |= std::exchange(appliedRegion, QRegion());

            if (!map->infinite())       // but keep within map boundaries
                region &= mapRect;
        }
    }

    for (auto& [before, target] : context.originalToOutputLayerMapping) {
        OutputLayerData &data = mExistingOutputTileLayers[before];

        // reduce memory usage by saving only diffs
        data.region = before->computeDiffRegion(*target);
        const QRect diffRect = data.region.boundingRect();

        auto beforeDiff = before->copy(data.region);
        beforeDiff->setPosition(diffRect.topLeft());
        beforeDiff->setName(before->name());

        auto afterDiff = target->copy(data.region);
        afterDiff->setPosition(diffRect.topLeft());
        afterDiff->setName(target->name());

        data.before = std::move(beforeDiff);
        data.after = std::move(afterDiff);
    }

    // Make sure to add any newly used tilesets to the map
    for (const SharedTileset &tileset : qAsConst(context.newTilesets))
        if (context.targetMap->isTilesetUsed(tileset.data()))
            new AddTileset(mapDocument, tileset, this);

    // Apply any property changes to existing layers
    QHashIterator<Layer*, Properties> changedPropertiesIt(context.changedProperties);
    while (changedPropertiesIt.hasNext()) {
        const auto item = changedPropertiesIt.next();
        new ChangeProperties(mapDocument, QString(), item.key(), item.value(), this);
    }

    // Add any new non-empty layers to the map
    auto newLayerIndex = context.targetMap->layerCount();
    for (auto &layer : context.newLayers) {
        if (layer->isTileLayer() && layer->isEmpty())
            continue;

        if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
            if (std::none_of(context.newMapObjects.cbegin(),
                             context.newMapObjects.cend(),
                             [=] (const AddMapObjects::Entry &entry) { return entry.objectGroup == objectGroup; })) {
                continue;
            }
        }

        new AddLayer(mapDocument,
                     newLayerIndex++,
                     layer.release(), nullptr, this);
    }

    // Add any newly placed objects
    if (!context.newMapObjects.isEmpty())
        new AddMapObjects(mapDocument, context.newMapObjects, this);

    // Remove any objects that have been scheduled for removal
    if (!context.mapObjectsToRemove.isEmpty())
        new RemoveMapObjects(mapDocument, context.mapObjectsToRemove.values(), this);
}

AutoMapperWrapper::~AutoMapperWrapper()
{
}

void AutoMapperWrapper::undo()
{
    for (const auto& [target, data] : mExistingOutputTileLayers)
        patchLayer(target, *data.before, data.region);

    QUndoCommand::undo(); // undo child commands
}

void AutoMapperWrapper::redo()
{
    QUndoCommand::redo(); // redo child commands

    for (const auto& [target, data] : mExistingOutputTileLayers)
        patchLayer(target, *data.after, data.region);
}

void AutoMapperWrapper::patchLayer(TileLayer *target, const TileLayer &layer, const QRegion &region)
{
    TilePainter painter(mMapDocument, target);
    painter.setCells(layer.x(), layer.y(), &layer, region);
}
