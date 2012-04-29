/*
 * quickstampmanager.cpp
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include "quickstampmanager.h"

#include "abstracttool.h"
#include "bucketfilltool.h"
#include "mapdocument.h"
#include "map.h"
#include "stampbrush.h"
#include "tilelayer.h"
#include "tileselectiontool.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "toolmanager.h"

using namespace Tiled;
using namespace Tiled::Internal;

QuickStampManager *QuickStampManager::mInstance = 0;

QuickStampManager *QuickStampManager::instance()
{
    if (!mInstance)
        mInstance = new QuickStampManager;
    return mInstance;
}

void QuickStampManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

QuickStampManager::QuickStampManager() :
    mMapDocument(0)
{
    mQuickStamps.resize(keys().length());
}

QuickStampManager::~QuickStampManager()
{
    cleanQuickStamps();
}

void QuickStampManager::saveQuickStamp(int index)
{
    if (!mMapDocument)
        return;

    const Map *map = mMapDocument->map();

    // The source of the saved stamp depends on which tool is selected
    AbstractTool *selectedTool = ToolManager::instance()->selectedTool();
    TileLayer *copy = 0;
    if (dynamic_cast<StampBrush*>(selectedTool)) {
        TileLayer *stamp = (static_cast<StampBrush*>(selectedTool))->stamp();
        if (!stamp)
            return;

        copy = static_cast<TileLayer*>(stamp->clone());
    } else if (dynamic_cast<TileSelectionTool*>(selectedTool)) {
        const TileLayer *tileLayer =
                dynamic_cast<TileLayer*>(mMapDocument->currentLayer());
        if (!tileLayer)
            return;

        const QRegion &selection = mMapDocument->tileSelection();
        if (selection.isEmpty())
            return;

        copy = tileLayer->copy(selection.translated(-tileLayer->x(),
                                                    -tileLayer->y()));
    } else if (dynamic_cast<BucketFillTool*>(selectedTool)) {
        TileLayer *stamp = (static_cast<BucketFillTool*>(selectedTool))->stamp();
        if (!stamp)
            return;

        copy = static_cast<TileLayer*>(stamp->clone());
    }

    if (!copy)
        return;

    Map *copyMap = new Map(map->orientation(),
                           copy->width(), copy->height(),
                           map->tileWidth(), map->tileHeight());

    copyMap->addLayer(copy);

    // Add tileset references to map and tileset manager
    TilesetManager *tilesetManager = TilesetManager::instance();
    foreach (Tileset *tileset, copy->usedTilesets()) {
        copyMap->addTileset(tileset);
        tilesetManager->addReference(tileset);
    }

    eraseQuickStamp(index);
    mQuickStamps.replace(index, copyMap);
}

void QuickStampManager::cleanQuickStamps()
{
    for (int i = 0; i < mQuickStamps.size(); i++)
        eraseQuickStamp(i);
}

void QuickStampManager::eraseQuickStamp(int index)
{
    if (Map *quickStamp = mQuickStamps.at(index)) {
        // Decrease reference to tilesets
        TilesetManager *tilesetManager = TilesetManager::instance();
        tilesetManager->removeReferences(quickStamp->tilesets());
        delete quickStamp;
    }
}

void QuickStampManager::selectQuickStamp(int index)
{
    if (!mMapDocument)
        return;

    if (Map *stampMap = mQuickStamps.at(index)) {
        mMapDocument->unifyTilesets(stampMap);
        emit setStampBrush(static_cast<TileLayer*>(stampMap->layerAt(0)));
    }
}

void QuickStampManager::setMapDocument(MapDocument *mapDocument)
{
    mMapDocument = mapDocument;
}
