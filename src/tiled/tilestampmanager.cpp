/*
 * tilestampmanager.cpp
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2014-2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilestampmanager.h"

#include "abstracttool.h"
#include "bucketfilltool.h"
#include "documentmanager.h"
#include "mapdocument.h"
#include "map.h"
#include "stampbrush.h"
#include "tilelayer.h"
#include "tileselectiontool.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "tilestamp.h"
#include "tilestampmodel.h"
#include "toolmanager.h"

using namespace Tiled;
using namespace Tiled::Internal;

TileStampManager::TileStampManager(const ToolManager &toolManager,
                                   QObject *parent)
    : QObject(parent)
    , mQuickStamps(quickStampKeys().length())
    , mTileStampModel(new TileStampModel(this))
    , mToolManager(toolManager)
{
}

TileStampManager::~TileStampManager()
{
    // needs to be over here where the TileStamp type is complete
}

static TileStamp stampFromContext(AbstractTool *selectedTool)
{
    TileStamp stamp;

    if (StampBrush *stampBrush = dynamic_cast<StampBrush*>(selectedTool)) {
        // take the stamp from the stamp brush
        stamp = stampBrush->stamp();
    } else if (BucketFillTool *fillTool = dynamic_cast<BucketFillTool*>(selectedTool)) {
        // take the stamp from the fill tool
        stamp = fillTool->stamp();
    } else if (MapDocument *mapDocument = DocumentManager::instance()->currentDocument()) {
        // try making a stamp from the current tile selection
        const TileLayer *tileLayer =
                dynamic_cast<TileLayer*>(mapDocument->currentLayer());
        if (!tileLayer)
            return stamp;

        QRegion selection = mapDocument->selectedArea();
        if (selection.isEmpty())
            return stamp;

        selection.translate(-tileLayer->position());
        QScopedPointer<TileLayer> copy(tileLayer->copy(selection));

        if (copy->size().isEmpty())
            return stamp;

        const Map *map = mapDocument->map();
        Map *copyMap = new Map(map->orientation(),
                               copy->width(), copy->height(),
                               map->tileWidth(), map->tileHeight());

        // Add tileset references to map
        foreach (const SharedTileset &tileset, copy->usedTilesets())
            copyMap->addTileset(tileset);

        copyMap->setRenderOrder(map->renderOrder());
        copyMap->addLayer(copy.take());

        stamp.addVariation(copyMap);
    }

    return stamp;
}

void TileStampManager::newStamp()
{
    TileStamp stamp = stampFromContext(mToolManager.selectedTool());
    if (stamp.isEmpty())
        return;

    mTileStampModel->addStamp(stamp);
}

void TileStampManager::addVariation(const TileStamp &targetStamp)
{
    TileStamp stamp = stampFromContext(mToolManager.selectedTool());
    if (stamp.isEmpty())
        return;

    if (stamp == targetStamp) // avoid easy mistake of adding duplicates
        return;

    foreach (const TileStampVariation &variation, stamp.variations())
        mTileStampModel->addVariation(targetStamp, variation);
}

void TileStampManager::saveQuickStamp(int index)
{
    TileStamp stamp = stampFromContext(mToolManager.selectedTool());
    if (stamp.isEmpty())
        return;

    saveQuickStamp(index, stamp);
}

void TileStampManager::extendQuickStamp(int index)
{
    TileStamp quickStamp = mQuickStamps[index];

    if (quickStamp.isEmpty())
        saveQuickStamp(index);
    else
        addVariation(quickStamp);
}

void TileStampManager::eraseQuickStamp(int index)
{
    const TileStamp stamp = mQuickStamps.at(index);
    if (!stamp.isEmpty()) {
        mQuickStamps[index] = TileStamp();

        if (!mQuickStamps.contains(stamp))
            mTileStampModel->removeStamp(stamp);
    }
}

void TileStampManager::saveQuickStamp(int index, TileStamp stamp)
{
    stamp.setName(tr("Quickstamp %1").arg(index + 1));
    stamp.setQuickStampIndex(index);

    // make sure existing quickstamp is removed from stamp model
    eraseQuickStamp(index);

    mTileStampModel->addStamp(stamp);

    mQuickStamps[index] = stamp;
}

void TileStampManager::selectQuickStamp(int index)
{
    const TileStamp &stamp = mQuickStamps.at(index);
    if (!stamp.isEmpty())
        emit setStamp(stamp);
}
