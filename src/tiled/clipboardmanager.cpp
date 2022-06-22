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
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectreferenceshelper.h"
#include "snaphelper.h"
#include "tile.h"
#include "tilelayer.h"
#include "tmxmapformat.h"

#include <QApplication>
#include <QClipboard>
#include <QHash>
#include <QJsonDocument>
#include <QMimeData>
#include <QSet>
#include <QUndoStack>

#include <QCborArray>
#include <QCborValue>

#include <algorithm>

static const char * const TMX_MIMETYPE = "text/tmx";

using namespace Tiled;

ClipboardManager::ClipboardManager()
    : mClipboard(QApplication::clipboard())
    , mHasMap(false)
    , mHasProperties(false)
{
    // Connection queued to avoid a potential crash in QXcbClipboard::mimeData
    // in direct response to QClipboard::emitChanged (see QTBUG-22552).
    connect(mClipboard, &QClipboard::dataChanged,
            this, &ClipboardManager::update,
            Qt::QueuedConnection);

    update();
}

/**
 * Returns the clipboard manager instance. Creates the instance when it
 * doesn't exist yet.
 */
ClipboardManager *ClipboardManager::instance()
{
    static ClipboardManager instance;
    return &instance;
}

/**
 * Retrieves the map from the clipboard. Returns null when there was no map or
 * loading failed.
 */
std::unique_ptr<Map> ClipboardManager::map() const
{
    const QMimeData *mimeData = mClipboard->mimeData();
    const QByteArray data = mimeData->data(QLatin1String(TMX_MIMETYPE));
    if (data.isEmpty())
        return nullptr;

    TmxMapFormat format;
    return format.fromByteArray(data);
}

/**
 * Sets the given map on the clipboard.
 */
void ClipboardManager::setMap(const Map &map)
{
    TmxMapFormat format;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QLatin1String(TMX_MIMETYPE), format.toByteArray(&map));

    mClipboard->setMimeData(mimeData);
}

Properties ClipboardManager::properties() const
{
    const QMimeData *mimeData = mClipboard->mimeData();
    const QByteArray data = mimeData->data(QLatin1String(PROPERTIES_MIMETYPE));

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    const QJsonArray array = QJsonDocument::fromBinaryData(data).array();
#else
    const QJsonArray array = QCborValue::fromCbor(data).toArray().toJsonArray();
#endif

    return propertiesFromJson(array);
}

void ClipboardManager::setProperties(const Properties &properties)
{
    QMimeData *mimeData = new QMimeData;

    const QJsonArray propertiesJson = propertiesToJson(properties);
    const QJsonDocument document(propertiesJson);

    mimeData->setText(QString::fromUtf8(document.toJson()));

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    mimeData->setData(QLatin1String(PROPERTIES_MIMETYPE), document.toBinaryData());
#else
    mimeData->setData(QLatin1String(PROPERTIES_MIMETYPE), QCborArray::fromJsonArray(propertiesJson).toCborValue().toCbor());
#endif

    mClipboard->setMimeData(mimeData);
}

/**
 * Convenience method to copy the current selection to the clipboard.
 * Copies selected tiles when any tile layer is selected and selected objects
 * when any object layer is selected.
 *
 * @returns whether anything was copied.
 */
bool ClipboardManager::copySelection(const MapDocument &mapDocument)
{
    const Map *map = mapDocument.map();
    const QRegion &selectedArea = mapDocument.selectedArea();
    const QList<MapObject*> selectedObjects = mapDocument.selectedObjectsOrdered();
    const QList<Layer*> selectedLayers = mapDocument.selectedLayersOrdered();

    const QRect selectionBounds = selectedArea.boundingRect();

    // Create a temporary map to write to the clipboard
    Map::Parameters mapParameters = map->parameters();
    mapParameters.width = selectionBounds.width();
    mapParameters.height = selectionBounds.height();
    mapParameters.infinite = false;
    Map copyMap(mapParameters);

    if (!selectedArea.isEmpty()) {
        bool tileLayerSelected = std::any_of(selectedLayers.begin(), selectedLayers.end(),
                                             [] (Layer *layer) { return layer->isTileLayer(); });

        if (tileLayerSelected)
            map->copyLayers(selectedLayers, selectedArea, copyMap);
    }

    if (!selectedObjects.isEmpty()) {
        bool objectGroupSelected = std::any_of(selectedLayers.begin(), selectedLayers.end(),
                                               [] (Layer *layer) { return layer->isObjectGroup(); });

        if (objectGroupSelected) {
            // Create a new object group with clones of the selected objects
            ObjectGroup *objectGroup = new ObjectGroup;
            for (const MapObject *mapObject : selectedObjects)
                objectGroup->addObject(mapObject->clone());
            copyMap.addLayer(objectGroup);
        }
    }

    if (copyMap.layerCount() > 0) {
        // Resolve the set of tilesets used by the created map
        copyMap.addTilesets(copyMap.usedTilesets());

        setMap(copyMap);
        return true;
    }

    return false;
}

/**
 * Convenience method that deals with some of the logic related to pasting
 * a group of objects.
 */
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
        // FIXME: This is not the visual center
        const QPointF center = objectGroup->objectsBoundingRect().center();

        // Take the mouse position if the mouse is on the view, otherwise
        // take the center of the view.
        QPoint viewPos;
        if (view->underMouse())
            viewPos = view->mapFromGlobal(QCursor::pos());
        else
            viewPos = QPoint(view->width() / 2, view->height() / 2);

        QPointF layerScreenPos = view->mapToScene(viewPos);
        layerScreenPos -= view->mapScene()->absolutePositionForLayer(*currentObjectGroup);

        insertPos = renderer->screenToPixelCoords(layerScreenPos) - center;
        SnapHelper(renderer).snap(insertPos);
    }

    QVector<AddMapObjects::Entry> objectsToAdd;
    objectsToAdd.reserve(objectGroup->objectCount());

    Map *map = mapDocument->map();
    ObjectReferencesHelper objectRefs(map);

    for (const MapObject *mapObject : objectGroup->objects()) {
        if (flags & PasteNoTileObjects && !mapObject->cell().isEmpty())
            continue;

        MapObject *objectClone = mapObject->clone();
        objectClone->setPosition(objectClone->position() + insertPos);

        objectRefs.reassignId(objectClone);

        objectsToAdd.append(AddMapObjects::Entry { objectClone, currentObjectGroup });
    }

    objectRefs.rewire();

    auto command = new AddMapObjects(mapDocument, objectsToAdd);
    command->setText(tr("Paste Objects"));

    mapDocument->undoStack()->push(command);
    mapDocument->setSelectedObjects(AddMapObjects::objects(objectsToAdd));
}

void ClipboardManager::update()
{
    bool hasMap = false;
    bool hasProperties = false;

    if (const QMimeData *data = mClipboard->mimeData()) {
        hasMap = data->hasFormat(QLatin1String(TMX_MIMETYPE));
        hasProperties = data->hasFormat(QLatin1String(PROPERTIES_MIMETYPE));
    }

    if (hasMap != mHasMap) {
        mHasMap = hasMap;
        emit hasMapChanged();
    }

    if (hasProperties != mHasProperties) {
        mHasProperties = hasProperties;
        emit hasPropertiesChanged();
    }
}

#include "moc_clipboardmanager.cpp"
