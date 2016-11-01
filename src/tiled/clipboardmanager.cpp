/*
 * clipboardmanager.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "clipboardmanager.h"

#include "addremovemapobject.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapview.h"
#include "objectgroup.h"
#include "snaphelper.h"
#include "tmxmapreader.h"
#include "tmxmapwriter.h"
#include "tile.h"
#include "tilelayer.h"

#include "rtbmapsettings.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QSet>
#include <QUndoStack>

#include <cmath>

static const char * const TMX_MIMETYPE = "text/tmx";

using namespace Tiled;
using namespace Tiled::Internal;

ClipboardManager *ClipboardManager::mInstance = 0;

ClipboardManager::ClipboardManager() :
    mHasMap(false)
  , mSelectedArea(QRegion())
  , mIsCut(false)
{
    mClipboard = QApplication::clipboard();
    connect(mClipboard, SIGNAL(dataChanged()), SLOT(updateHasMap()));

    updateHasMap();
}

ClipboardManager *ClipboardManager::instance()
{
    if (!mInstance)
        mInstance = new ClipboardManager;
    return mInstance;
}

void ClipboardManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

Map *ClipboardManager::map() const
{
    const QMimeData *mimeData = mClipboard->mimeData();
    const QByteArray data = mimeData->data(QLatin1String(TMX_MIMETYPE));
    if (data.isEmpty())
        return 0;

    TmxMapReader reader;
    return reader.fromByteArray(data);
}

void ClipboardManager::setMap(const Map *map)
{
    TmxMapWriter mapWriter;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QLatin1String(TMX_MIMETYPE), mapWriter.toByteArray(map));

    mClipboard->setMimeData(mimeData);
}

void ClipboardManager::copySelection(const MapDocument *mapDocument)
{
    const Layer *currentLayer = mapDocument->currentLayer();
    if (!currentLayer)
        return;

    const Map *map = mapDocument->map();
    const QRegion &selectedArea = mapDocument->selectedArea();
    const QList<MapObject*> &selectedObjects = mapDocument->selectedObjects();
    const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(currentLayer);
    Layer *copyLayer = 0;

    if (!selectedArea.isEmpty() && tileLayer) {
        // Copy the selected part of the layer
        copyLayer = tileLayer->copy(selectedArea.translated(-tileLayer->x(),
                                                             -tileLayer->y()));
    } else if (!selectedObjects.isEmpty()) {
        // Create a new object group with clones of the selected objects
        ObjectGroup *objectGroup = new ObjectGroup;
        foreach (const MapObject *mapObject, selectedObjects)
        {
            MapObject *objectClone = mapObject->clone();
            objectGroup->addObject(objectClone);
            objectClone->rtbMapObject()->setOriginID(mapObject->id());
        }

        copyLayer = objectGroup;
    } else {
        return;
    }

    // Create a temporary map to write to the clipboard
    Map copyMap(map->orientation(),
                copyLayer->width(), copyLayer->height(),
                map->tileWidth(), map->tileHeight());

    copyMap.setRenderOrder(map->renderOrder());

    // Resolve the set of tilesets used by this layer
    foreach (const SharedTileset &tileset, copyLayer->usedTilesets())
        copyMap.addTileset(tileset);

    copyMap.addLayer(copyLayer);

    setMap(&copyMap);

    emit copyAreaChanged();
}

void ClipboardManager::copySelectionAllLayers(const MapDocument *mapDocument)
{
    const Layer *currentLayer = mapDocument->currentLayer();
    if (!currentLayer)
        return;

    const Map *map = mapDocument->map();
    const QRegion &selectedArea = mapDocument->selectedArea();
    Layer *copyFloorLayer = 0;
    Layer *copyObjectLayer = 0;
    Layer *copyOrbLayer = 0;

    if (!selectedArea.isEmpty())
    {
        mSelectedArea = selectedArea;

        const TileLayer *floorLayer = map->layerAt(RTBMapSettings::FloorID)->asTileLayer();
        // Copy the selected part of the layer
        copyFloorLayer = floorLayer->copy(selectedArea.translated(-floorLayer->x(),
                                                             -floorLayer->y()));

        QList<MapObject*> objects = map->objectGroups().at(0)->objects();
        QList<MapObject*> orbs = map->objectGroups().at(1)->objects();

        copyObjectLayer = copyObjectInArea(mapDocument, objects);
        copyOrbLayer = copyObjectInArea(mapDocument, orbs);
    }

    // Create a temporary map to write to the clipboard
    Map copyMap(map->orientation(),
                copyFloorLayer->width(), copyFloorLayer->height(),
                map->tileWidth(), map->tileHeight());

    copyMap.setRenderOrder(map->renderOrder());

    // Resolve the set of tilesets used by this layer
    foreach (const SharedTileset &tileset, copyFloorLayer->usedTilesets())
        copyMap.addTileset(tileset);

    copyMap.addLayer(copyFloorLayer);
    copyMap.addLayer(copyObjectLayer);
    copyMap.addLayer(copyOrbLayer);

    setMap(&copyMap);

    noteRelatedObjects(mapDocument, copyObjectLayer->asObjectGroup());

    emit copyAreaChanged();
}

void ClipboardManager::pasteObjectGroup(const ObjectGroup *objectGroup,
                                        MapDocument *mapDocument,
                                        const MapView *view,
                                        PasteMode mode)
{
    Layer *currentLayer = mapDocument->currentLayer();
    if (!currentLayer)
        return;

    ObjectGroup *currentObjectGroup = currentLayer->asObjectGroup();
    if (!currentObjectGroup)
        return;

    // Determine where to insert the objects
    const MapRenderer *renderer = mapDocument->renderer();
    const QPointF center = objectGroup->objectsBoundingRect().center();

    // Take the mouse position if the mouse is on the view, otherwise
    // take the center of the view.
    QPoint viewPos;
    if (view->underMouse())
        viewPos = view->mapFromGlobal(QCursor::pos());
    else
        viewPos = QPoint(view->width() / 2, view->height() / 2);

    QPointF scenePos = view->mapToScene(viewPos);
    // set the position to the correct cursor position (shift one cell down)
    scenePos.setY(scenePos.y() + 32);

    QPointF insertPos = renderer->screenToPixelCoords(scenePos) - center;
    SnapHelper(renderer).snap(insertPos);

    QUndoStack *undoStack = mapDocument->undoStack();
    QList<MapObject*> pastedObjects;
    pastedObjects.reserve(objectGroup->objectCount());

    mPastedObjects.clear();

    undoStack->beginMacro(tr("Paste Objects"));
    foreach (const MapObject *mapObject, objectGroup->objects()) {
        if (mode == NoTileObjects && !mapObject->cell().isEmpty())
            continue;

        // continue if the object is not allowed in the current layer
        if(!checkCurrentLayer(mapDocument, mapObject))
            continue;

        // add object only if the new position is on the map
        QPointF newPosition = mapObject->position() + insertPos;
        if(pasteInMap(mapDocument, newPosition))
        {
            MapObject *objectClone = mapObject->clone();
            objectClone->setPosition(newPosition);
            pastedObjects.append(objectClone);
            undoStack->push(new AddMapObject(mapDocument,
                                             currentObjectGroup,
                                             objectClone));

            mPastedObjects.append(objectClone);
        }
    }

    restoreConnections(mapDocument);

    undoStack->endMacro();

    mapDocument->setSelectedObjects(pastedObjects);
}

void ClipboardManager::pasteAllObjectGroups(MapDocument *mapDocument,
                                        const MapView *view,
                                        PasteMode mode)
{
    if (!hasMap())
        return;

    ObjectGroup *objectLayer = map()->layerAt(RTBMapSettings::ObjectID)->asObjectGroup();
    ObjectGroup *orbLayer = map()->layerAt(RTBMapSettings::OrbObjectID)->asObjectGroup();

    SharedTileset tileset = mapDocument->map()->tilesetAt(0);
    SharedTileset objectLayerTileset;
    SharedTileset orbLayerTileset;

    if(!objectLayer->usedTilesets().isEmpty())
        objectLayerTileset = objectLayer->usedTilesets().values().first();

    if(!orbLayer->usedTilesets().isEmpty())
        orbLayerTileset = orbLayer->usedTilesets().values().first();

    // update tileset references
    if(objectLayerTileset && objectLayerTileset != tileset)
        objectLayer->replaceReferencesToTileset(objectLayerTileset.data(), tileset.data());

    if(orbLayerTileset && orbLayerTileset != tileset)
        orbLayer->replaceReferencesToTileset(orbLayerTileset.data(), tileset.data());


    // find the correct center of the selected area
    QPointF centerSelectedArea = mSelectedArea.boundingRect().topLeft();
    centerSelectedArea.setX(centerSelectedArea.x() + (int) std::floor(mSelectedArea.boundingRect().width() / 2));
    centerSelectedArea.setY(centerSelectedArea.y() + (int) std::floor(mSelectedArea.boundingRect().height() / 2));
    centerSelectedArea = centerSelectedArea * map()->tileWidth();

    // Take the mouse position if the mouse is on the view, otherwise
    // take the center of the view.
    QPoint viewPos;
    if (view->underMouse())
        viewPos = view->mapFromGlobal(QCursor::pos());
    else
        viewPos = QPoint(view->width() / 2, view->height() / 2);

    QPointF scenePos = view->mapToScene(viewPos);
    // prevent right/bottom shift of the target cell
    scenePos.setX((int) std::round(scenePos.x()) - 1);
    scenePos.setY((int) std::round(scenePos.y()) - 1);

    QUndoStack *undoStack = mapDocument->undoStack();
    mPastedObjects.clear();

    QList<MapObject *> objects = objectLayer->objects();
    for(int i = 0; i < objects.size(); i ++)
    {
    //foreach (MapObject *mapObject, objectLayer->objects()) {
        MapObject *mapObject = objects.at(i);
        if (mode == NoTileObjects && !mapObject->cell().isEmpty())
            continue;

        MapObject *objectClone = pasteObject(mapDocument, mapObject, scenePos, undoStack
                    , mapDocument->map()->layerAt(RTBMapSettings::ObjectID)->asObjectGroup()
                    , centerSelectedArea);

        mPastedObjects.append(objectClone);
    }

    foreach (MapObject *mapObject, orbLayer->objects()) {
        if (mode == NoTileObjects && !mapObject->cell().isEmpty())
            continue;

         pasteObject(mapDocument, mapObject, scenePos, undoStack
                    , mapDocument->map()->layerAt(RTBMapSettings::OrbObjectID)->asObjectGroup(),
                    centerSelectedArea);
    }

    restoreConnections(mapDocument);

    undoStack->endMacro();
}

void ClipboardManager::restoreConnections(MapDocument *mapDocument)
{
    for(MapObject *obj : mPastedObjects)
    {
        RTBMapObject* rtbObject = obj->rtbMapObject();
        switch (rtbObject->objectType()) {
        case RTBMapObject::Button:
        {
            RTBButtonObject *button = static_cast<RTBButtonObject*>(rtbObject);
            if(!button->targets().isEmpty())
            {
                bool isTargetPasted;
                int target;
                for(int i = RTBMapObject::Target1; i <= RTBMapObject::Target5; i++)
                {
                    isTargetPasted = false;
                    target = button->target(i).toInt();
                    if(target != 0)
                    {
                        for(MapObject *obj : mPastedObjects)
                        {
                            if(obj->rtbMapObject()->originID() == target)
                            {
                                button->insertTarget(i, QString::number(obj->id()));
                                isTargetPasted = true;
                                break;
                            }
                        }

                        if(!isTargetPasted)
                        {
                            button->insertTarget(i, QLatin1String(""));
                        }
                    }
                }

            }

            break;
        }
        case RTBMapObject::Teleporter:
        {
            RTBTeleporter *teleporter = static_cast<RTBTeleporter*>(obj->rtbMapObject());
            int target = teleporter->teleporterTarget().toInt();
            if(target != 0)
            {
                for(MapObject *obj : mPastedObjects)
                {
                    if(obj->rtbMapObject()->originID() == target)
                    {
                        teleporter->setTeleporterTarget(QString::number(obj->id()));
                        break;
                    }
                }
            }

            break;
        }
        case RTBMapObject::CameraTrigger:
        {
            RTBCameraTrigger *camera = static_cast<RTBCameraTrigger*>(obj->rtbMapObject());
            int target = camera->target().toInt();
            if(target != 0)
            {
                for(MapObject *obj : mPastedObjects)
                {
                    if(obj->rtbMapObject()->originID() == target)
                    {
                        camera->setTarget(QString::number(obj->id()));
                        break;
                    }
                }
            }

            break;
        }
        // update related objects
        case RTBMapObject::Target:
        {
            if(mIsCut)
            {
                for(MapObject *object : mRelatedObjects)
                {
                    RTBMapObject *rtbObject = object->rtbMapObject();
                    switch (rtbObject->objectType()) {
                    case RTBMapObject::Teleporter:
                    {
                        RTBTeleporter *teleporter = static_cast<RTBTeleporter*>(rtbObject);
                        if(teleporter->teleporterTarget().toInt() == obj->rtbMapObject()->originID())
                        {
                            for(MapObject *mapObject : mapDocument->map()->objectGroups().first()->objects())
                            {
                                if(mapObject->id() == teleporter->originID())
                                {
                                    RTBTeleporter *originTeleporter = static_cast<RTBTeleporter*>(mapObject->rtbMapObject());

                                    if(originTeleporter->teleporterTarget().isEmpty())
                                    {
                                        originTeleporter->setTeleporterTarget(QString::number(obj->id()));
                                        break;
                                    }
                                }
                            }
                        }

                        break;
                    }
                    case RTBMapObject::CameraTrigger:
                    {
                        RTBCameraTrigger *camera = static_cast<RTBCameraTrigger*>(rtbObject);
                        if(camera->target().toInt() == obj->rtbMapObject()->originID())
                        {
                            for(MapObject *mapObject : mapDocument->map()->objectGroups().first()->objects())
                            {
                                if(mapObject->id() == camera->originID())
                                {
                                    RTBCameraTrigger *originCamera = static_cast<RTBCameraTrigger*>(mapObject->rtbMapObject());
                                    if(originCamera->target().isEmpty())
                                    {
                                        originCamera->setTarget(QString::number(obj->id()));
                                        break;
                                    }
                                }
                            }
                        }

                        break;
                    }

                    }
                }
            }
        }
        case RTBMapObject::LaserBeam:
        {
            if(mIsCut)
            {
                for(MapObject *object : mRelatedObjects)
                {
                    RTBMapObject *rtbObject = object->rtbMapObject();
                    switch (rtbObject->objectType())
                    {
                        case RTBMapObject::Button:
                        {
                            RTBButtonObject *button = static_cast<RTBButtonObject*>(rtbObject);
                            if(button->containsTarget(QString::number(obj->rtbMapObject()->originID())))
                            {
                                for(MapObject *mapObject : mapDocument->map()->objectGroups().first()->objects())
                                {
                                    if(mapObject->id() == button->originID())
                                    {
                                        RTBButtonObject *originButton = static_cast<RTBButtonObject*>(mapObject->rtbMapObject());
                                        originButton->appendTarget(QString::number(obj->id()));
                                    }
                                }
                            }

                            break;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

MapObject *ClipboardManager::pasteObject(MapDocument *mapDocument, MapObject *mapObject, QPointF insertPos
                                        , QUndoStack *undoStack, ObjectGroup *objectGroup, QPointF areaCenter)
{
    const MapRenderer *renderer = mapDocument->renderer();
    int tileWidth = map()->tileWidth();

    // convert insert/areaCenter position to whole tiles
    insertPos.setX((int) std::floor(insertPos.x() / tileWidth));
    insertPos.setY((int) std::floor(insertPos.y() / tileWidth));
    insertPos = renderer->tileToScreenCoords(insertPos);

    areaCenter.setX((int) std::floor(areaCenter.x() / tileWidth));
    areaCenter.setY((int) std::floor(areaCenter.y() / tileWidth));
    areaCenter = renderer->tileToScreenCoords(areaCenter);

    // difference between object position and center of the selected area
    QPointF diff = areaCenter - mapObject->position();
    QPointF newPosition = insertPos - diff;
    // convert to tile
    newPosition = renderer->screenToTileCoords(newPosition);
    // convert to screen coords
    newPosition = renderer->tileToScreenCoords(newPosition);

    if(pasteInMap(mapDocument, newPosition))
    {
        MapObject *objectClone = mapObject->clone();
        objectClone->setPosition(newPosition);
        undoStack->push(new AddMapObject(mapDocument,
                                        objectGroup,
                                        objectClone));

       return objectClone;
    }

    return 0;
}

bool ClipboardManager::pasteInMap(MapDocument *mapDocument, QPointF position)
{
    if(position.x() / 32 >= 0 && position.y() / 32 >= 0
            && position.x() / 32 < mapDocument->map()->width()
            && position.y() / 32 <= mapDocument->map()->height())
    {
        return true;
    }
    else
        return false;
}

void ClipboardManager::updateHasMap()
{
    const QMimeData *data = mClipboard->mimeData();
    const bool mapInClipboard =
            data && data->hasFormat(QLatin1String(TMX_MIMETYPE));

    if (mapInClipboard != mHasMap) {
        mHasMap = mapInClipboard;
        emit hasMapChanged();
    }
}

bool ClipboardManager::checkCurrentLayer(MapDocument *mapDocument, const MapObject *mapObject)
{
    int layerIndex = mapDocument->currentLayerIndex();
    int type = mapObject->rtbMapObject()->objectType();

    if(layerIndex == RTBMapSettings::ObjectID && type > RTBMapSettings::OrbBorder && type <= RTBMapSettings::ObjectBorder)
        return true;
    else if(layerIndex == RTBMapSettings::OrbObjectID && type == RTBMapObject::Orb)
        return true;

    return false;
}

ObjectGroup *ClipboardManager::copyObjectInArea(const MapDocument *mapDocument, QList<MapObject*> objects)
{
    ObjectGroup *copyObjectGroup = new ObjectGroup;

    const QRegion &selectedArea = mapDocument->selectedArea();
    const MapRenderer *renderer = mapDocument->renderer();

    for(MapObject *obj : objects)
    {
        for(QRect rect : selectedArea.rects())
        {
            // move the rect 1 cell to the right and 1 down
            QPointF topLeft = rect.topLeft();
            rect.setTopLeft(QPoint(topLeft.x() + 1, topLeft.y() + 1));
            QPointF bottomRight = rect.bottomRight();
            rect.setBottomRight(QPoint(bottomRight.x() + 1, bottomRight.y() + 1));

            if(rect.contains(renderer->screenToTileCoords(obj->boundsUseTile().center()).toPoint()))
            {
                MapObject *objectClone = obj->clone();
                objectClone->rtbMapObject()->setOriginID(obj->id());
                copyObjectGroup->addObject(objectClone);
            }
        }
    }

    return copyObjectGroup;
}

void ClipboardManager::noteRelatedObjects(const MapDocument *mapDocument, ObjectGroup *copyObjectLayer)
{
    mRelatedObjects.clear();

    for(MapObject *mapObject : mapDocument->map()->objectGroups().first()->objects())
    {
        RTBMapObject* rtbObject = mapObject->rtbMapObject();
        switch (rtbObject->objectType()) {
        case RTBMapObject::Teleporter:
        {
            RTBTeleporter *teleporter = static_cast<RTBTeleporter*>(rtbObject);
            if(!teleporter->teleporterTarget().isEmpty())
            {
                int target = teleporter->teleporterTarget().toInt();
                for(MapObject *obj : copyObjectLayer->asObjectGroup()->objects())
                {
                    if(obj->rtbMapObject()->originID() == target)
                    {
                        MapObject *objectClone = mapObject->clone();
                        objectClone->rtbMapObject()->setOriginID(mapObject->id());
                        mRelatedObjects.append(objectClone);
                    }
                }
            }
            break;
        }
        case RTBMapObject::CameraTrigger:
        {
            RTBCameraTrigger *camera = static_cast<RTBCameraTrigger*>(rtbObject);
            if(!camera->target().isEmpty())
            {
                int target = camera->target().toInt();
                for(MapObject *obj : copyObjectLayer->asObjectGroup()->objects())
                {
                    if(obj->rtbMapObject()->originID() == target)
                    {
                        MapObject *objectClone = mapObject->clone();
                        objectClone->rtbMapObject()->setOriginID(mapObject->id());
                        mRelatedObjects.append(objectClone);
                    }
                }
            }
            break;
        }
        case RTBMapObject::Button:
        {
            RTBButtonObject *button = static_cast<RTBButtonObject*>(rtbObject);
            if(!button->targets().isEmpty())
            {
                for(MapObject *obj : copyObjectLayer->asObjectGroup()->objects())
                {
                    if(button->containsTarget(QString::number(obj->rtbMapObject()->originID())))
                    {
                        MapObject *objectClone = mapObject->clone();
                        objectClone->rtbMapObject()->setOriginID(mapObject->id());
                        mRelatedObjects.append(objectClone);
                    }
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void ClipboardManager::clear()
{
    mClipboard->clear();
}
