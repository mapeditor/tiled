/*
 * mapdocument.cpp
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
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

#include "mapdocument.h"

#include "addremovelayer.h"
#include "addremovemapobject.h"
#include "addremovetileset.h"
#include "changeproperties.h"
#include "changeselectedarea.h"
#include "containerhelpers.h"
#include "flipmapobjects.h"
#include "hexagonalrenderer.h"
#include "imagelayer.h"
#include "isometricrenderer.h"
#include "layermodel.h"
#include "mapobjectmodel.h"
#include "map.h"
#include "mapobject.h"
#include "movelayer.h"
#include "movemapobject.h"
#include "movemapobjecttogroup.h"
#include "objectgroup.h"
#include "offsetlayer.h"
#include "orthogonalrenderer.h"
#include "painttilelayer.h"
#include "pluginmanager.h"
#include "resizemap.h"
#include "resizetilelayer.h"
#include "rotatemapobject.h"
#include "staggeredrenderer.h"
#include "terrain.h"
#include "terrainmodel.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilesetmanager.h"
#include "tmxmapformat.h"

#include <QFileInfo>
#include <QRect>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

MapDocument::MapDocument(Map *map, const QString &fileName)
    : Document(fileName)
    , mMap(map)
    , mLayerModel(new LayerModel(this))
    , mCurrentObject(map)
    , mRenderer(nullptr)
    , mMapObjectModel(new MapObjectModel(this))
    , mTerrainModel(new TerrainModel(this, this))
{
    createRenderer();

    mCurrentLayerIndex = (map->layerCount() == 0) ? -1 : 0;
    mLayerModel->setMapDocument(this);

    // Forward signals emitted from the layer model
    connect(mLayerModel, &LayerModel::layerAdded,
            this, &MapDocument::onLayerAdded);
    connect(mLayerModel, &LayerModel::layerAboutToBeRemoved,
            this, &MapDocument::onLayerAboutToBeRemoved);
    connect(mLayerModel, &LayerModel::layerRemoved,
            this, &MapDocument::onLayerRemoved);
    connect(mLayerModel, &LayerModel::layerChanged,
            this, &MapDocument::layerChanged);

    // Forward signals emitted from the map object model
    mMapObjectModel->setMapDocument(this);
    connect(mMapObjectModel, SIGNAL(objectsAdded(QList<MapObject*>)),
            SIGNAL(objectsAdded(QList<MapObject*>)));
    connect(mMapObjectModel, SIGNAL(objectsChanged(QList<MapObject*>)),
            SIGNAL(objectsChanged(QList<MapObject*>)));
    connect(mMapObjectModel, SIGNAL(objectsRemoved(QList<MapObject*>)),
            SLOT(onObjectsRemoved(QList<MapObject*>)));

    connect(mMapObjectModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(onMapObjectModelRowsInserted(QModelIndex,int,int)));
    connect(mMapObjectModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(onMapObjectModelRowsInsertedOrRemoved(QModelIndex,int,int)));
    connect(mMapObjectModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            SLOT(onObjectsMoved(QModelIndex,int,int,QModelIndex,int)));

    connect(mTerrainModel, SIGNAL(terrainRemoved(Terrain*)),
            SLOT(onTerrainRemoved(Terrain*)));

    // Register tileset references
    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReferences(mMap->tilesets());
}

MapDocument::~MapDocument()
{
    // Unregister tileset references
    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->removeReferences(mMap->tilesets());

    delete mRenderer;
    delete mMap;
}

bool MapDocument::save(QString *error)
{
    return save(fileName(), error);
}

bool MapDocument::save(const QString &fileName, QString *error)
{
    MapFormat *mapFormat = mWriterFormat;

    TmxMapFormat tmxMapFormat;
    if (!mapFormat)
        mapFormat = &tmxMapFormat;

    if (!mapFormat->write(map(), fileName)) {
        if (error)
            *error = mapFormat->errorString();
        return false;
    }

    undoStack()->setClean();
    setFileName(fileName);
    mLastSaved = QFileInfo(fileName).lastModified();

    emit saved();
    return true;
}

MapDocument *MapDocument::load(const QString &fileName,
                               MapFormat *mapFormat,
                               QString *error)
{
    TmxMapFormat tmxMapFormat;

    if (!mapFormat && !tmxMapFormat.supportsFile(fileName)) {
        // Try to find a plugin that implements support for this format
        auto formats = PluginManager::objects<MapFormat>();
        for (MapFormat *format : formats) {
            if (format->supportsFile(fileName)) {
                mapFormat = format;
                break;
            }
        }
    }

    Map *map = nullptr;
    QString errorString;
    if (mapFormat) {
        map = mapFormat->read(fileName);
        errorString = mapFormat->errorString();
    } else {
        map = tmxMapFormat.read(fileName);
        errorString = tmxMapFormat.errorString();
    }

    if (!map) {
        if (error)
            *error = errorString;
        return nullptr;
    }

    MapDocument *mapDocument = new MapDocument(map, fileName);
    if (mapFormat) {
        mapDocument->setReaderFormat(mapFormat);
        if (mapFormat->hasCapabilities(MapFormat::Write))
            mapDocument->setWriterFormat(mapFormat);
    }
    return mapDocument;
}

MapFormat *MapDocument::readerFormat() const
{
    return mReaderFormat;
}

void MapDocument::setReaderFormat(MapFormat *format)
{
    mReaderFormat = format;
}

MapFormat *MapDocument::writerFormat() const
{
    return mWriterFormat;
}

void MapDocument::setWriterFormat(MapFormat *format)
{
    mWriterFormat = format;
}

MapFormat *MapDocument::exportFormat() const
{
    return mExportFormat;
}

void MapDocument::setExportFormat(MapFormat *format)
{
    mExportFormat = format;
}

/**
 * Returns the name with which to display this map. It is the file name without
 * its path, or 'untitled.tmx' when the map has no file name.
 */
QString MapDocument::displayName() const
{
    QString displayName = QFileInfo(mFileName).fileName();
    if (displayName.isEmpty())
        displayName = tr("untitled.tmx");

    return displayName;
}

void MapDocument::setCurrentLayerIndex(int index)
{
    Q_ASSERT(index >= -1 && index < mMap->layerCount());

    const bool changed = mCurrentLayerIndex != index;
    mCurrentLayerIndex = index;

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
    emit currentLayerIndexChanged(mCurrentLayerIndex);

    if (changed && mCurrentLayerIndex != -1)
        setCurrentObject(currentLayer());
}

Layer *MapDocument::currentLayer() const
{
    if (mCurrentLayerIndex == -1)
        return nullptr;

    return mMap->layerAt(mCurrentLayerIndex);
}

/**
 * Custom intersects check necessary because QRectF::intersects wants a
 * non-empty area of overlap, but we should also consider overlap with empty
 * area as intersection.
 *
 * Results for rectangles with negative size are undefined.
 */
static bool intersects(const QRectF &a, const QRectF &b)
{
    return a.right() >= b.left() &&
            a.bottom() >= b.top() &&
            a.left() <= b.right() &&
            a.top() <= b.bottom();
}

static bool visibleIn(const QRectF &area, MapObject *object,
                      MapRenderer *renderer)
{
    QRectF boundingRect = renderer->boundingRect(object);

    if (object->rotation() != 0) {
        // Rotate around object position
        QPointF pos = renderer->pixelToScreenCoords(object->position());
        boundingRect.translate(-pos);

        QTransform transform;
        transform.rotate(object->rotation());
        boundingRect = transform.mapRect(boundingRect);

        boundingRect.translate(pos);
    }

    return intersects(area, boundingRect);
}

void MapDocument::resizeMap(const QSize &size, const QPoint &offset)
{
    const QRegion movedSelection = mSelectedArea.translated(offset);
    const QRect newArea = QRect(-offset, size);
    const QRectF visibleArea = mRenderer->boundingRect(newArea);

    const QPointF origin = mRenderer->tileToPixelCoords(QPointF());
    const QPointF newOrigin = mRenderer->tileToPixelCoords(-offset);
    const QPointF pixelOffset = origin - newOrigin;

    // Resize the map and each layer
    mUndoStack->beginMacro(tr("Resize Map"));
    for (int i = 0; i < mMap->layerCount(); ++i) {
        Layer *layer = mMap->layerAt(i);

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            TileLayer *tileLayer = static_cast<TileLayer*>(layer);
            mUndoStack->push(new ResizeTileLayer(this, tileLayer, size, offset));
            break;
        }
        case Layer::ObjectGroupType: {
            ObjectGroup *objectGroup = static_cast<ObjectGroup*>(layer);

            // Remove objects that will fall outside of the map
            foreach (MapObject *o, objectGroup->objects()) {
                if (!visibleIn(visibleArea, o, mRenderer)) {
                    mUndoStack->push(new RemoveMapObject(this, o));
                } else {
                    QPointF oldPos = o->position();
                    QPointF newPos = oldPos + pixelOffset;
                    mUndoStack->push(new MoveMapObject(this, o, newPos, oldPos));
                }
            }
            break;
        }
        case Layer::ImageLayerType:
            // Currently not adjusted when resizing the map
            break;
        }
    }

    mUndoStack->push(new ResizeMap(this, size));
    mUndoStack->push(new ChangeSelectedArea(this, movedSelection));
    mUndoStack->endMacro();

    // TODO: Handle layers that don't match the map size correctly
}

void MapDocument::offsetMap(const QList<int> &layerIndexes,
                            const QPoint &offset,
                            const QRect &bounds,
                            bool wrapX, bool wrapY)
{
    if (layerIndexes.empty())
        return;

    if (layerIndexes.size() == 1) {
        mUndoStack->push(new OffsetLayer(this, layerIndexes.first(), offset,
                                         bounds, wrapX, wrapY));
    } else {
        mUndoStack->beginMacro(tr("Offset Map"));
        foreach (const int layerIndex, layerIndexes) {
            mUndoStack->push(new OffsetLayer(this, layerIndex, offset,
                                             bounds, wrapX, wrapY));
        }
        mUndoStack->endMacro();
    }
}

/**
 * Flips the selected objects in the given \a direction.
 */
void MapDocument::flipSelectedObjects(FlipDirection direction)
{
    if (mSelectedObjects.isEmpty())
        return;

    mUndoStack->push(new FlipMapObjects(this, mSelectedObjects, direction));
}

/**
 * Rotates the selected objects.
 */
void MapDocument::rotateSelectedObjects(RotateDirection direction)
{
    if (mSelectedObjects.isEmpty())
        return;

    mUndoStack->beginMacro(tr("Rotate %n Object(s)", "",
                              mSelectedObjects.size()));

    // TODO: Rotate them properly as a group
    foreach (MapObject *mapObject, mSelectedObjects) {
        const qreal oldRotation = mapObject->rotation();
        qreal newRotation = oldRotation;

        if (direction == RotateLeft) {
            newRotation -= 90;
            if (newRotation < -180)
                newRotation += 360;
        } else {
            newRotation += 90;
            if (newRotation > 180)
                newRotation -= 360;
        }

        mUndoStack->push(new RotateMapObject(this, mapObject,
                                             newRotation, oldRotation));
    }
    mUndoStack->endMacro();
}

/**
 * Adds a layer of the given type to the top of the layer stack. After adding
 * the new layer, emits editLayerNameRequested().
 */
void MapDocument::addLayer(Layer::TypeFlag layerType)
{
    Layer *layer = nullptr;
    QString name;

    switch (layerType) {
    case Layer::TileLayerType:
        name = tr("Tile Layer %1").arg(mMap->tileLayerCount() + 1);
        layer = new TileLayer(name, 0, 0, mMap->width(), mMap->height());
        break;
    case Layer::ObjectGroupType:
        name = tr("Object Layer %1").arg(mMap->objectGroupCount() + 1);
        layer = new ObjectGroup(name, 0, 0, mMap->width(), mMap->height());
        break;
    case Layer::ImageLayerType:
        name = tr("Image Layer %1").arg(mMap->imageLayerCount() + 1);
        layer = new ImageLayer(name, 0, 0, mMap->width(), mMap->height());
        break;
    }
    Q_ASSERT(layer);

    const int index = mMap->layerCount();
    mUndoStack->push(new AddLayer(this, index, layer));
    setCurrentLayerIndex(index);

    emit editLayerNameRequested();
}

/**
 * Duplicates the currently selected layer.
 */
void MapDocument::duplicateLayer()
{
    if (mCurrentLayerIndex == -1)
        return;

    Layer *duplicate = mMap->layerAt(mCurrentLayerIndex)->clone();
    duplicate->setName(tr("Copy of %1").arg(duplicate->name()));

    const int index = mCurrentLayerIndex + 1;
    QUndoCommand *cmd = new AddLayer(this, index, duplicate);
    cmd->setText(tr("Duplicate Layer"));
    mUndoStack->push(cmd);
    setCurrentLayerIndex(index);
}

/**
 * Merges the currently selected layer with the layer below. This only works
 * when the layers can be merged.
 *
 * \see Layer::canMergeWith
 */
void MapDocument::mergeLayerDown()
{
    if (mCurrentLayerIndex < 1)
        return;

    Layer *upperLayer = mMap->layerAt(mCurrentLayerIndex);
    Layer *lowerLayer = mMap->layerAt(mCurrentLayerIndex - 1);

    if (!lowerLayer->canMergeWith(upperLayer))
        return;

    Layer *merged = lowerLayer->mergedWith(upperLayer);

    mUndoStack->beginMacro(tr("Merge Layer Down"));
    mUndoStack->push(new AddLayer(this, mCurrentLayerIndex - 1, merged));
    mUndoStack->push(new RemoveLayer(this, mCurrentLayerIndex));
    mUndoStack->push(new RemoveLayer(this, mCurrentLayerIndex));
    mUndoStack->endMacro();
}

/**
 * Moves the given layer up. Does nothing when no valid layer index is
 * given.
 */
void MapDocument::moveLayerUp(int index)
{
    if (index < 0 || index >= mMap->layerCount() - 1)
        return;

    mUndoStack->push(new MoveLayer(this, index, MoveLayer::Up));
}

/**
 * Moves the given layer down. Does nothing when no valid layer index is
 * given.
 */
void MapDocument::moveLayerDown(int index)
{
    if (index < 1 || index >= mMap->layerCount())
        return;

    mUndoStack->push(new MoveLayer(this, index, MoveLayer::Down));
}

/**
 * Removes the given layer.
 */
void MapDocument::removeLayer(int index)
{
    if (index < 0 || index >= mMap->layerCount())
        return;

    mUndoStack->push(new RemoveLayer(this, index));
}

/**
  * Show or hide all other layers except the layer at the given index.
  * If any other layer is visible then all layers will be hidden, otherwise
  * the layers will be shown.
  */
void MapDocument::toggleOtherLayers(int index)
{
    mLayerModel->toggleOtherLayers(index);
}

/**
 * Adds a tileset to this map at the given \a index. Emits the appropriate
 * signal.
 */
void MapDocument::insertTileset(int index, const SharedTileset &tileset)
{
    emit tilesetAboutToBeAdded(index);
    mMap->insertTileset(index, tileset);
    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReference(tileset);
    emit tilesetAdded(index, tileset.data());
}

static bool isFromTileset(Object *object, Tileset *tileset)
{
    if (!object)
        return false;

    if (object->typeId() == Object::TileType
            && tileset == static_cast<Tile*>(object)->tileset())
        return true;

    if (object->typeId() == Object::TerrainType
            && tileset == static_cast<Terrain*>(object)->tileset())
        return true;

    return false;
}

/**
 * Removes the tileset at the given \a index from this map. Emits the
 * appropriate signal.
 *
 * \warning Does not make sure that any references to tiles in the removed
 *          tileset are cleared.
 */
void MapDocument::removeTilesetAt(int index)
{
    emit tilesetAboutToBeRemoved(index);

    SharedTileset tileset = mMap->tilesets().at(index);

    if (tileset.data() == mCurrentObject || isFromTileset(mCurrentObject, tileset.data()))
        setCurrentObject(nullptr);

    mMap->removeTilesetAt(index);
    emit tilesetRemoved(tileset.data());

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->removeReference(tileset);
}

void MapDocument::moveTileset(int from, int to)
{
    if (from == to)
        return;

    SharedTileset tileset = mMap->tilesets().at(from);
    mMap->removeTilesetAt(from);
    mMap->insertTileset(to, tileset);
    emit tilesetMoved(from, to);
}

/**
 * Replaces the tileset at the given \a index with the new \a tileset. Replaces
 * all tiles from the replaced tileset with tiles from the new tileset.
 *
 * @return The replaced tileset.
 */
SharedTileset MapDocument::replaceTileset(int index, const SharedTileset &tileset)
{
    SharedTileset oldTileset = mMap->tilesetAt(index);

    mMap->replaceTileset(oldTileset, tileset);

    // Try to keep the current object valid
    if (mCurrentObject == oldTileset.data()) {
        setCurrentObject(tileset.data());

    } else if (mCurrentObject && mCurrentObject->typeId() == Object::TileType) {
        Tile *tile = static_cast<Tile*>(mCurrentObject);
        if (tile->tileset() == oldTileset.data())
            setCurrentObject(tileset->findTile(tile->id()));

    } else if (mCurrentObject && mCurrentObject->typeId() == Object::TerrainType) {
        Terrain *terrain = static_cast<Terrain*>(mCurrentObject);
        if (terrain->tileset() == oldTileset.data())
            setCurrentObject(tileset->terrain(terrain->id()));
    }


    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReference(tileset);
    tilesetManager->removeReference(oldTileset);

    emit tilesetReplaced(index, tileset.data());

    return oldTileset;
}

void MapDocument::setSelectedArea(const QRegion &selection)
{
    if (mSelectedArea != selection) {
        const QRegion oldSelectedArea = mSelectedArea;
        mSelectedArea = selection;
        emit selectedAreaChanged(mSelectedArea, oldSelectedArea);
    }
}

void MapDocument::setSelectedObjects(const QList<MapObject *> &selectedObjects)
{
    mSelectedObjects = selectedObjects;
    emit selectedObjectsChanged();

    if (selectedObjects.size() == 1)
        setCurrentObject(selectedObjects.first());
}

void MapDocument::setSelectedTiles(const QList<Tile*> &selectedTiles)
{
    mSelectedTiles = selectedTiles;
    emit selectedTilesChanged();
}

void MapDocument::setCurrentObject(Object *object)
{
    if (object == mCurrentObject)
        return;

    mCurrentObject = object;
    emit currentObjectChanged(object);
}

QList<Object*> MapDocument::currentObjects() const
{
    QList<Object*> objects;
    if (mCurrentObject) {
        if (mCurrentObject->typeId() == Object::MapObjectType && !mSelectedObjects.isEmpty()) {
            const auto &selectedObjects = mSelectedObjects;
            for (MapObject *mapObj : selectedObjects)
                objects.append(mapObj);
        } else if (mCurrentObject->typeId() == Object::TileType && !mSelectedTiles.isEmpty()) {
            const auto &selectedTiles = mSelectedTiles;
            for (Tile *tile : selectedTiles)
                objects.append(tile);
        } else {
            objects.append(mCurrentObject);
        }
    }
    return objects;
}

/**
 * Makes sure the all tilesets which are used at the given \a map will be
 * present in the map document.
 *
 * To reach the aim, all similar tilesets will be replaced by the version
 * in the current map document and all missing tilesets will be added to
 * the current map document.
 *
 * \warning This method assumes that the tilesets in \a map are managed by
 *          the TilesetManager!
 */
void MapDocument::unifyTilesets(Map *map)
{
    QList<QUndoCommand*> undoCommands;
    const QVector<SharedTileset> &existingTilesets = mMap->tilesets();
    TilesetManager *tilesetManager = TilesetManager::instance();

    // Add tilesets that are not yet part of this map
    for (const SharedTileset &tileset : map->tilesets()) {
        if (existingTilesets.contains(tileset))
            continue;

        SharedTileset replacement = tileset->findSimilarTileset(existingTilesets);
        if (!replacement) {
            undoCommands.append(new AddTileset(this, tileset));
            continue;
        }

        // Merge the tile properties
        for (Tile *replacementTile : replacement->tiles()) {
            if (Tile *originalTile = tileset->findTile(replacementTile->id())) {
                Properties properties = replacementTile->properties();
                properties.merge(originalTile->properties());
                undoCommands.append(new ChangeProperties(this,
                                                         tr("Tile"),
                                                         replacementTile,
                                                         properties));
            }
        }

        tilesetManager->addReference(replacement);
        tilesetManager->removeReference(tileset);

        map->replaceTileset(tileset, replacement);
    }

    if (!undoCommands.isEmpty()) {
        mUndoStack->beginMacro(tr("Tileset Changes"));
        foreach (QUndoCommand *command, undoCommands)
            mUndoStack->push(command);
        mUndoStack->endMacro();
    }
}

/**
 * Replaces tilesets in \a map by similar tilesets in this map when possible,
 * and adds tilesets to \a missingTilesets whenever there is a tileset without
 * replacement in this map.
 *
 * \warning This method assumes that the tilesets in \a map are managed by
 *          the TilesetManager!
 */
void MapDocument::unifyTilesets(Map *map, QVector<SharedTileset> &missingTilesets)
{
    const QVector<SharedTileset> &existingTilesets = mMap->tilesets();
    TilesetManager *tilesetManager = TilesetManager::instance();

    for (const SharedTileset &tileset : map->tilesets()) {
        // tileset already added
        if (existingTilesets.contains(tileset))
            continue;

        SharedTileset replacement = tileset->findSimilarTileset(existingTilesets);

        // tileset not present and no replacement tileset found
        if (!replacement) {
            if (!missingTilesets.contains(tileset))
                missingTilesets.append(tileset);
            continue;
        }

        // replacement tileset found, change given map
        tilesetManager->addReference(replacement);
        tilesetManager->removeReference(tileset);

        map->replaceTileset(tileset, replacement);
    }
}

/**
 * Emits the tileset changed signal. This signal is currently used when adding
 * or removing tiles from a tileset.
 *
 * @todo Emit more specific signals.
 */
void MapDocument::emitTilesetChanged(Tileset *tileset)
{
    Q_ASSERT(contains(mMap->tilesets(), tileset));
    emit tilesetChanged(tileset);
}

/**
 * Before forwarding the signal, the objects are removed from the list of
 * selected objects, triggering a selectedObjectsChanged signal when
 * appropriate.
 */
void MapDocument::onObjectsRemoved(const QList<MapObject*> &objects)
{
    deselectObjects(objects);
    emit objectsRemoved(objects);
}

void MapDocument::onMapObjectModelRowsInserted(const QModelIndex &parent,
                                               int first, int last)
{
    ObjectGroup *objectGroup = mMapObjectModel->toObjectGroup(parent);
    if (!objectGroup) // we're not dealing with insertion of objects
        return;

    emit objectsInserted(objectGroup, first, last);
    onMapObjectModelRowsInsertedOrRemoved(parent, first, last);
}

void MapDocument::onMapObjectModelRowsInsertedOrRemoved(const QModelIndex &parent,
                                                        int first, int last)
{
    Q_UNUSED(first)

    ObjectGroup *objectGroup = mMapObjectModel->toObjectGroup(parent);
    if (!objectGroup)
        return;

    // Inserting or removing objects changes the index of any that come after
    const int lastIndex = objectGroup->objectCount() - 1;
    if (last < lastIndex)
        emit objectsIndexChanged(objectGroup, last + 1, lastIndex);
}

void MapDocument::onObjectsMoved(const QModelIndex &parent, int start, int end,
                                 const QModelIndex &destination, int row)
{
    if (parent != destination)
        return;

    ObjectGroup *objectGroup = mMapObjectModel->toObjectGroup(parent);

    // Determine the full range over which object indexes changed
    const int first = qMin(start, row);
    const int last = qMax(end, row - 1);

    emit objectsIndexChanged(objectGroup, first, last);
}

void MapDocument::onLayerAdded(int index)
{
    emit layerAdded(index);

    // Select the first layer that gets added to the map
    if (mMap->layerCount() == 1)
        setCurrentLayerIndex(0);
}

void MapDocument::onLayerAboutToBeRemoved(int index)
{
    Layer *layer = mMap->layerAt(index);
    if (layer == mCurrentObject)
        setCurrentObject(nullptr);

    // Deselect any objects on this layer when necessary
    if (ObjectGroup *og = dynamic_cast<ObjectGroup*>(layer))
        deselectObjects(og->objects());
    emit layerAboutToBeRemoved(index);
}

void MapDocument::onLayerRemoved(int index)
{
    // Bring the current layer index to safety
    bool currentLayerAffected = index <= mCurrentLayerIndex;
    if (currentLayerAffected)
        mCurrentLayerIndex = mCurrentLayerIndex - 1;

    emit layerRemoved(index);

    // Emitted after the layerRemoved signal so that the MapScene has a chance
    // of synchronizing before adapting to the newly selected index
    if (currentLayerAffected)
        emit currentLayerIndexChanged(mCurrentLayerIndex);
}

void MapDocument::onTerrainRemoved(Terrain *terrain)
{
    if (terrain == mCurrentObject)
        setCurrentObject(nullptr);
}

void MapDocument::deselectObjects(const QList<MapObject *> &objects)
{
    // Unset the current object when it was part of this list of objects
    if (mCurrentObject && mCurrentObject->typeId() == Object::MapObjectType)
        if (objects.contains(static_cast<MapObject*>(mCurrentObject)))
            setCurrentObject(nullptr);

    int removedCount = 0;
    foreach (MapObject *object, objects)
        removedCount += mSelectedObjects.removeAll(object);

    if (removedCount > 0)
        emit selectedObjectsChanged();
}

void MapDocument::setTilesetFileName(Tileset *tileset,
                                     const QString &fileName)
{
    tileset->setFileName(fileName);
    emit tilesetFileNameChanged(tileset);
}

void MapDocument::setTilesetName(Tileset *tileset, const QString &name)
{
    tileset->setName(name);
    emit tilesetNameChanged(tileset);
}

void MapDocument::setTilesetTileOffset(Tileset *tileset,
                                       const QPoint &tileOffset)
{
    tileset->setTileOffset(tileOffset);
    mMap->recomputeDrawMargins();
    emit tilesetTileOffsetChanged(tileset);
}

void MapDocument::duplicateObjects(const QList<MapObject *> &objects)
{
    if (objects.isEmpty())
        return;

    mUndoStack->beginMacro(tr("Duplicate %n Object(s)", "", objects.size()));

    QList<MapObject*> clones;
    foreach (const MapObject *mapObject, objects) {
        MapObject *clone = mapObject->clone();
        clones.append(clone);
        mUndoStack->push(new AddMapObject(this,
                                          mapObject->objectGroup(),
                                          clone));
    }

    mUndoStack->endMacro();
    setSelectedObjects(clones);
}

void MapDocument::removeObjects(const QList<MapObject *> &objects)
{
    if (objects.isEmpty())
        return;

    mUndoStack->beginMacro(tr("Remove %n Object(s)", "", objects.size()));
    foreach (MapObject *mapObject, objects)
        mUndoStack->push(new RemoveMapObject(this, mapObject));
    mUndoStack->endMacro();
}

void MapDocument::moveObjectsToGroup(const QList<MapObject *> &objects,
                                     ObjectGroup *objectGroup)
{
    if (objects.isEmpty())
        return;

    mUndoStack->beginMacro(tr("Move %n Object(s) to Layer", "",
                              objects.size()));

    foreach (MapObject *mapObject, objects) {
        if (mapObject->objectGroup() == objectGroup)
            continue;

        mUndoStack->push(new MoveMapObjectToGroup(this,
                                                  mapObject,
                                                  objectGroup));
    }
    mUndoStack->endMacro();
}

void MapDocument::setProperty(Object *object,
                              const QString &name,
                              const QString &value)
{
    const bool hadProperty = object->hasProperty(name);
    object->setProperty(name, value);

    if (hadProperty)
        emit propertyChanged(object, name);
    else
        emit propertyAdded(object, name);
}

void MapDocument::setProperties(Object *object, const Properties &properties)
{
    object->setProperties(properties);
    emit propertiesChanged(object);
}

void MapDocument::removeProperty(Object *object, const QString &name)
{
    object->removeProperty(name);
    emit propertyRemoved(object, name);
}

void MapDocument::createRenderer()
{
    if (mRenderer)
        delete mRenderer;

    switch (mMap->orientation()) {
    case Map::Isometric:
        mRenderer = new IsometricRenderer(mMap);
        break;
    case Map::Staggered:
        mRenderer = new StaggeredRenderer(mMap);
        break;
    case Map::Hexagonal:
        mRenderer = new HexagonalRenderer(mMap);
        break;
    default:
        mRenderer = new OrthogonalRenderer(mMap);
        break;
    }
}
