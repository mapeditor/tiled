/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "clipboardmanager.h"

#include "map.h"
#include "mapdocument.h"
#include "tmxmapreader.h"
#include "tmxmapwriter.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileselectionmodel.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QSet>

static const char * const TMX_MIMETYPE = "text/tmx";

using namespace Tiled;
using namespace Tiled::Internal;

ClipboardManager::ClipboardManager(QObject *parent) :
    QObject(parent),
    mHasMap(false)
{
    mClipboard = QApplication::clipboard();
    connect(mClipboard, SIGNAL(dataChanged()), SLOT(updateHasMap()));

    updateHasMap();
}

Map *ClipboardManager::map() const
{
    const QMimeData *mimeData = mClipboard->mimeData();
    const QByteArray data = mimeData->data(QLatin1String(TMX_MIMETYPE));
    if (data.isEmpty())
        return 0;

    const QString mapData = QString::fromUtf8(data);
    TmxMapReader reader;
    return reader.fromString(mapData);
}

void ClipboardManager::setMap(const Map *map)
{
    TmxMapWriter mapWriter;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QLatin1String(TMX_MIMETYPE),
                      mapWriter.toString(map).toUtf8());

    mClipboard->setMimeData(mimeData);
}

void ClipboardManager::copySelection(const MapDocument *mapDocument)
{
    int currentLayer = mapDocument->currentLayer();
    if (currentLayer == -1)
        return;

    const Map *map = mapDocument->map();
    const Layer *layer = map->layerAt(currentLayer);
    const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
    if (!tileLayer)
        return;

    const QRegion &selection = mapDocument->selectionModel()->selection();
    if (selection.isEmpty())
        return;

    // Copy the selected part of the layer
    TileLayer *copy = tileLayer->copy(selection.translated(-tileLayer->x(),
                                                           -tileLayer->y()));

    // Create a temporary map to write to the clipboard
    Map copyMap(copy->width(), copy->height(),
                map->tileWidth(), map->tileHeight());

    // Resolve the set of tilesets used by this layer
    QSet<Tileset*> tilesets;
    for (int y = 0; y < copy->height(); ++y) {
        for (int x = 0; x < copy->width(); ++x) {
            const Tile *tile = copy->tileAt(x, y);
            if (tile)
                tilesets.insert(tile->tileset());
        }
    }
    foreach (Tileset *tileset, tilesets)
        copyMap.addTileset(tileset);

    copyMap.addLayer(copy);

    setMap(&copyMap);
}

void ClipboardManager::updateHasMap()
{
    const QMimeData *data = mClipboard->mimeData();
    const bool mapInClipboard = data->hasFormat(QLatin1String(TMX_MIMETYPE));

    if (mapInClipboard != mHasMap) {
        mHasMap = mapInClipboard;
        emit hasMapChanged();
    }
}
