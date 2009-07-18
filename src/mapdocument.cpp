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

#include "deletelayer.h"
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
    mLayerModel->setMapDocument(this);

    // Forward signals emitted from the layer model
    connect(mLayerModel, SIGNAL(layerAdded(int)), SLOT(onLayerAdded(int)));
    connect(mLayerModel, SIGNAL(layerRemoved(int)), SLOT(onLayerRemoved(int)));
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
    Q_ASSERT(index >= -1 && index < mMap->layers().size());
    mCurrentLayer = index;

    /* This function always sends the following signal, even if the index
     * didn't actually change. This is because the selected index in the layer
     * table view might be out of date anyway, and would otherwise not be
     * properly updated.
     *
     * This problem happens due to the selection model not sending signals
     * about changes to its current index when it is due to insertion/removal
     * of other items. The selected item doesn't change in that case, but our
     * layer index does.
     */
    emit currentLayerChanged(mCurrentLayer);
}

int MapDocument::currentLayer() const
{
    return mCurrentLayer;
}

/**
 * Moves the given layer up. Does nothing when no valid layer index is
 * given.
 */
void MapDocument::moveLayerUp(int index)
{
    if (index < 0 || index >= mMap->layers().size() - 1)
        return;

    mUndoStack->push(new MoveLayer(this, index, MoveLayer::Up));
}

/**
 * Moves the given layer down. Does nothing when no valid layer index is
 * given.
 */
void MapDocument::moveLayerDown(int index)
{
    if (index < 1 || index >= mMap->layers().size())
        return;

    mUndoStack->push(new MoveLayer(this, index, MoveLayer::Down));
}

/**
 * Deletes the given layer.
 */
void MapDocument::deleteLayer(int index)
{
    if (index < 0 || index >= mMap->layers().size())
        return;

    mUndoStack->push(new DeleteLayer(this, index));
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

void MapDocument::onLayerAdded(int index)
{
    emit layerAdded(index);

    // Select the first layer that gets added to the map
    if (mMap->layers().size() == 1)
        setCurrentLayer(0);
}

void MapDocument::onLayerRemoved(int index)
{
    // Bring the current layer index to safety
    if (mCurrentLayer == mMap->layers().size())
        setCurrentLayer(mCurrentLayer - 1);

    emit layerRemoved(index);
}
