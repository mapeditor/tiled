/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#include "mapdocument.h"

#include "layertablemodel.h"
#include "map.h"
#include "movelayer.h"
#include "tileselectionmodel.h"
#include "tilesetmanager.h"

#include <QRect>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

MapDocument::MapDocument(Map *map):
    mMap(map),
    mLayerModel(new LayerTableModel(this)),
    mSelectionModel(new TileSelectionModel(this, this)),
    mUndoStack(new QUndoStack(this))
{
    mCurrentLayer = (map->layers().isEmpty()) ? -1 : 0;
    mLayerModel->setMap(mMap);

    // Forward signals emitted from the layer model
    connect(mLayerModel, SIGNAL(layerAdded(int)), SIGNAL(layerAdded(int)));
    connect(mLayerModel, SIGNAL(layerRemoved(int)), SIGNAL(layerRemoved(int)));
    connect(mLayerModel, SIGNAL(layerChanged(int)), SIGNAL(layerChanged(int)));

    // Register tileset references
    TilesetManager *tilesetManager = TilesetManager::instance();
    foreach (Tileset *tileset, mMap->tilesets().values())
        tilesetManager->addReference(tileset);
}

MapDocument::~MapDocument()
{
    // Unregister tileset references
    TilesetManager *tilesetManager = TilesetManager::instance();
    foreach (Tileset *tileset, mMap->tilesets().values())
        tilesetManager->removeReference(tileset);

    delete mMap;
}

void MapDocument::setCurrentLayer(int index)
{
    if (index == mCurrentLayer)
        return;

    mCurrentLayer = index;
    emit currentLayerChanged(mCurrentLayer);
}

int MapDocument::currentLayer() const
{
    return mCurrentLayer;
}

void MapDocument::moveLayerUp(int index)
{
    if (index < 0 || index >= mMap->layers().size() - 1)
        return;

    mUndoStack->push(new MoveLayer(this, index, MoveLayer::Up));
}

void MapDocument::moveLayerDown(int index)
{
    if (index < 1)
        return;

    mUndoStack->push(new MoveLayer(this, index, MoveLayer::Down));
}

void MapDocument::addTileset(Tileset *tileset)
{
    mMap->addTileset(tileset, 0);
    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReference(tileset);
    emit tilesetAdded(tileset);
}

void MapDocument::emitRegionChanged(const QRegion &region)
{
    emit regionChanged(region);
}

/**
 * Emits the objects added signal with the specified list of objects.
 * This will cause the scene to insert the related items.
 */
void MapDocument::emitObjectsAdded(const QList<MapObject*> &objects)
{
    emit objectsAdded(objects);
}

/**
 * Emits the objects removed signal with the specified list of objects.
 * This will cause the scene to remove the related items.
 */
void MapDocument::emitObjectsRemoved(const QList<MapObject*> &objects)
{
    emit objectsRemoved(objects);
}

/**
 * Emits the objects changed signal with the specified list of objects.
 * This will cause the scene to update the related items.
 */
void MapDocument::emitObjectsChanged(const QList<MapObject*> &objects)
{
    emit objectsChanged(objects);
}

QRect MapDocument::toPixelCoordinates(const QRect &r) const
{
    const int tileWidth = mMap->tileWidth();
    const int tileHeight = mMap->tileHeight();
    return QRect(r.x() * tileWidth,
                 r.y() * tileHeight,
                 r.width() * tileWidth,
                 r.height() * tileHeight);
}
