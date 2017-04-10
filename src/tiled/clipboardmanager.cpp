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
#include "tmxmapformat.h"
#include "tile.h"
#include "tilelayer.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QSet>
#include <QUndoStack>

static const char * const TMX_MIMETYPE = "text/tmx";

using namespace Tiled;
using namespace Tiled::Internal;

ClipboardManager *ClipboardManager::mInstance;

ClipboardManager::ClipboardManager() :
    mHasMap(false)
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
    mInstance = nullptr;
}

Map *ClipboardManager::map() const
{
    const QMimeData *mimeData = mClipboard->mimeData();
    const QByteArray data = mimeData->data(QLatin1String(TMX_MIMETYPE));
    if (data.isEmpty())
        return nullptr;

    TmxMapFormat format;
    return format.fromByteArray(data);
}

void ClipboardManager::setMap(const Map &map)
{
    TmxMapFormat format;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QLatin1String(TMX_MIMETYPE), format.toByteArray(&map));

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
    Layer *copyLayer = nullptr;

    if (!selectedArea.isEmpty() && tileLayer) {
        const QRegion area = selectedArea.intersected(tileLayer->bounds());

        // Copy the selected part of the layer
        copyLayer = tileLayer->copy(area.translated(-tileLayer->position()));
        copyLayer->setPosition(area.boundingRect().topLeft());

    } else if (!selectedObjects.isEmpty()) {
        // Create a new object group with clones of the selected objects
        ObjectGroup *objectGroup = new ObjectGroup;
        for (const MapObject *mapObject : selectedObjects)
            objectGroup->addObject(mapObject->clone());
        copyLayer = objectGroup;
    } else {
        return;
    }

    // Create a temporary map to write to the clipboard
    Map copyMap(map->orientation(),
                0, 0,
                map->tileWidth(), map->tileHeight());

    copyMap.setRenderOrder(map->renderOrder());

    // Resolve the set of tilesets used by this layer
    foreach (const SharedTileset &tileset, copyLayer->usedTilesets())
        copyMap.addTileset(tileset);

    copyMap.addLayer(copyLayer);

    setMap(copyMap);
}

void ClipboardManager::pasteObjectGroup(const ObjectGroup *objectGroup,
                                        MapDocument *mapDocument,
                                        const MapView *view,
                                        PasteFlags flags)
{
    Layer *currentLayer = mapDocument->currentLayer();
    if (!currentLayer)
        return;

    ObjectGroup *currentObjectGroup = currentLayer->asObjectGroup();
    if (!currentObjectGroup)
        return;

    QPointF insertPos;

    if (!(flags & PasteInPlace)) {
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

        const QPointF scenePos = view->mapToScene(viewPos);

        insertPos = renderer->screenToPixelCoords(scenePos) - center;
        SnapHelper(renderer).snap(insertPos);
    }

    QUndoStack *undoStack = mapDocument->undoStack();
    QList<MapObject*> pastedObjects;
    pastedObjects.reserve(objectGroup->objectCount());

    undoStack->beginMacro(tr("Paste Objects"));
    for (const MapObject *mapObject : objectGroup->objects()) {
        if (flags & PasteNoTileObjects && !mapObject->cell().isEmpty())
            continue;

        MapObject *objectClone = mapObject->clone();
        objectClone->resetId();
        objectClone->setPosition(objectClone->position() + insertPos);
        pastedObjects.append(objectClone);
        undoStack->push(new AddMapObject(mapDocument,
                                         currentObjectGroup,
                                         objectClone));
    }
    undoStack->endMacro();

    mapDocument->setSelectedObjects(pastedObjects);
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
