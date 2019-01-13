/*
 * mapscene.cpp
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "mapscene.h"

#include "abstracttool.h"
#include "addremovemapobject.h"
#include "containerhelpers.h"
#include "documentmanager.h"
#include "map.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "preferences.h"
#include "stylehelper.h"
#include "templatemanager.h"
#include "tilesetmanager.h"
#include "toolmanager.h"
#include "worldmanager.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QPalette>

#include "qtcompat_p.h"

using namespace Tiled;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMapDocument(nullptr),
    mSelectedTool(nullptr),
    mActiveTool(nullptr),
    mUnderMouse(false),
    mCurrentModifiers(Qt::NoModifier)
{
    updateDefaultBackgroundColor();

    connect(StyleHelper::instance(), &StyleHelper::styleApplied,
            this, &MapScene::updateDefaultBackgroundColor);

    TilesetManager *tilesetManager = TilesetManager::instance();
    connect(tilesetManager, &TilesetManager::tilesetImagesChanged,
            this, &MapScene::repaintTileset);
    connect(tilesetManager, &TilesetManager::repaintTileset,
            this, &MapScene::repaintTileset);

    WorldManager &worldManager = WorldManager::instance();
    connect(&worldManager, &WorldManager::worldsChanged, this, &MapScene::refreshScene);

    // Install an event filter so that we can get key events on behalf of the
    // active tool without having to have the current focus.
    qApp->installEventFilter(this);
}

MapScene::~MapScene()
{
    qApp->removeEventFilter(this);
}

/**
 * Sets the map this scene displays.
 */
void MapScene::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, &MapDocument::mapChanged,
                this, &MapScene::mapChanged);
        connect(mMapDocument, &MapDocument::tilesetTileOffsetChanged,
                this, &MapScene::adaptToTilesetTileSizeChanges);
        connect(mMapDocument, &MapDocument::tileImageSourceChanged,
                this, &MapScene::adaptToTileSizeChanges);
        connect(mMapDocument, &MapDocument::tilesetReplaced,
                this, &MapScene::tilesetReplaced);
    }

    refreshScene();
    emit mapDocumentChanged(mMapDocument);
}

/**
 * Returns the bounding rect of the map. This can be different from the
 * sceneRect() when multiple maps are displayed.
 */
QRectF MapScene::mapBoundingRect() const
{
    if (auto mapItem = mMapItems.value(mMapDocument))
        return mapItem->boundingRect();
    return QRectF();
}

/**
 * Sets the currently selected tool.
 */
void MapScene::setSelectedTool(AbstractTool *tool)
{
    mSelectedTool = tool;
}

/**
 * Refreshes the map scene.
 */
void MapScene::refreshScene()
{
    QHash<MapDocument*, MapItem*> mapItems;

    if (!mMapDocument) {
        mMapItems.swap(mapItems);
        qDeleteAll(mapItems);
        updateSceneRect();
        return;
    }

    WorldManager &worldManager = WorldManager::instance();

    if (const World *world = worldManager.worldForMap(mMapDocument->fileName())) {
        const QPoint currentMapPosition = world->mapRect(mMapDocument->fileName()).topLeft();
        auto const contextMaps = world->contextMaps(mMapDocument->fileName());

        for (const World::MapEntry &mapEntry : contextMaps) {
            MapDocumentPtr mapDocument;

            if (mapEntry.fileName == mMapDocument->fileName()) {
                mapDocument = mMapDocument->sharedFromThis();
            } else {
                auto doc = DocumentManager::instance()->loadDocument(mapEntry.fileName);
                mapDocument = doc.objectCast<MapDocument>();
            }

            if (mapDocument) {
                MapItem::DisplayMode displayMode = MapItem::ReadOnly;
                if (mapDocument == mMapDocument)
                    displayMode = MapItem::Editable;

                auto mapItem = takeOrCreateMapItem(mapDocument, displayMode);
                mapItem->setPos(mapEntry.rect.topLeft() - currentMapPosition);
                mapItems.insert(mapDocument.data(), mapItem);
            }
        }
    } else {
        auto mapItem = takeOrCreateMapItem(mMapDocument->sharedFromThis(), MapItem::Editable);
        mapItems.insert(mMapDocument, mapItem);
    }

    mMapItems.swap(mapItems);
    qDeleteAll(mapItems);       // delete all map items that didn't get reused

    updateSceneRect();

    const Map *map = mMapDocument->map();

    if (map->backgroundColor().isValid())
        setBackgroundBrush(map->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);
}

void MapScene::updateDefaultBackgroundColor()
{
    mDefaultBackgroundColor = QGuiApplication::palette().dark().color();

    if (!mMapDocument || !mMapDocument->map()->backgroundColor().isValid())
        setBackgroundBrush(mDefaultBackgroundColor);
}

void MapScene::updateSceneRect()
{
    QRectF sceneRect;

    for (MapItem *mapItem : qAsConst(mMapItems))
        sceneRect |= mapItem->boundingRect().translated(mapItem->pos());

    setSceneRect(sceneRect);
}

MapItem *MapScene::takeOrCreateMapItem(const MapDocumentPtr &mapDocument, MapItem::DisplayMode displayMode)
{
    // Try to reuse an existing map item
    auto mapItem = mMapItems.take(mapDocument.data());
    if (!mapItem) {
        mapItem = new MapItem(mapDocument, displayMode);
        connect(mapItem, &MapItem::boundingRectChanged, this, &MapScene::updateSceneRect);
        addItem(mapItem);
    } else {
        mapItem->setDisplayMode(displayMode);
    }
    return mapItem;
}

/**
 * Enables the selected tool at this map scene.
 * Therefore it tells that tool, that this is the active map scene.
 */
void MapScene::enableSelectedTool()
{
    if (!mSelectedTool || !mMapDocument)
        return;

    mActiveTool = mSelectedTool;
    mActiveTool->activate(this);

    mCurrentModifiers = QApplication::keyboardModifiers();
    mActiveTool->modifiersChanged(mCurrentModifiers);

    if (mUnderMouse) {
        mActiveTool->mouseEntered();
        mActiveTool->mouseMoved(mLastMousePos, mCurrentModifiers);
    }
}

void MapScene::disableSelectedTool()
{
    if (!mActiveTool)
        return;

    if (mUnderMouse)
        mActiveTool->mouseLeft();
    mActiveTool->deactivate(this);
    mActiveTool = nullptr;
}

/**
 * Updates the possibly changed background color.
 */
void MapScene::mapChanged()
{
    const Map *map = mMapDocument->map();
    if (map->backgroundColor().isValid())
        setBackgroundBrush(map->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);
}

void MapScene::repaintTileset(Tileset *tileset)
{
    for (MapItem *mapItem : qAsConst(mMapItems)) {
        if (contains(mapItem->mapDocument()->map()->tilesets(), tileset)) {
            update();
            return;
        }
    }
}

/**
 * This function should be called when any tiles in the given tileset may have
 * changed their size or offset or image.
 */
void MapScene::adaptToTilesetTileSizeChanges()
{
    update();
}

void MapScene::adaptToTileSizeChanges()
{
    update();
}

void MapScene::tilesetReplaced()
{
    adaptToTilesetTileSizeChanges();
}

/**
 * Override for handling enter and leave events.
 */
bool MapScene::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        mUnderMouse = true;
        if (mActiveTool)
            mActiveTool->mouseEntered();
        break;
    case QEvent::Leave:
        mUnderMouse = false;
        if (mActiveTool)
            mActiveTool->mouseLeft();
        break;
    default:
        break;
    }

    return QGraphicsScene::event(event);
}

void MapScene::keyPressEvent(QKeyEvent *event)
{
    if (mActiveTool)
        mActiveTool->keyPressed(event);

    if (!(mActiveTool && event->isAccepted()))
        QGraphicsScene::keyPressEvent(event);
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    mLastMousePos = mouseEvent->scenePos();

    if (!mMapDocument)
        return;

    QGraphicsScene::mouseMoveEvent(mouseEvent);

    // Currently we always want to inform the active tool about mouse move
    // events, regardless of whether this event was delived to a graphics item
    // as a hover event. This is due to the behavior of MapItem, which needs
    // to accept hover events but should not block them here.
//    if (mouseEvent->isAccepted())
//        return;

    if (mActiveTool) {
        mActiveTool->mouseMoved(mouseEvent->scenePos(),
                                mouseEvent->modifiers());
        mouseEvent->accept();
    }
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mousePressEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mActiveTool) {
        mouseEvent->accept();
        mActiveTool->mousePressed(mouseEvent);
    }
}

void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mActiveTool) {
        mouseEvent->accept();
        mActiveTool->mouseReleased(mouseEvent);
    }
}

void MapScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mActiveTool) {
        mouseEvent->accept();
        mActiveTool->mouseDoubleClicked(mouseEvent);
    }
}

static const ObjectTemplate *readObjectTemplate(const QMimeData *mimeData)
{
    if (!mimeData->hasFormat(QLatin1String(TEMPLATES_MIMETYPE)))
        return nullptr;

    QByteArray encodedData = mimeData->data(QLatin1String(TEMPLATES_MIMETYPE));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    QString fileName;
    stream >> fileName;

    return TemplateManager::instance()->findObjectTemplate(fileName);
}

/**
 * Override to ignore drag enter events except for templates.
 */
void MapScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->ignore();    // ignore, because events start out accepted

    if (!mapDocument())
        return;

    ObjectGroup *objectGroup = dynamic_cast<ObjectGroup*>(mapDocument()->currentLayer());
    if (!objectGroup)
        return;

    const ObjectTemplate *objectTemplate = readObjectTemplate(event->mimeData());
    if (!objectTemplate || !mapDocument()->templateAllowed(objectTemplate))
        return;

    QGraphicsScene::dragEnterEvent(event);  // accepts the event
}

/**
 * Accepts dropping a single template into an object group
 */
void MapScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!mapDocument())
        return;

    ObjectGroup *objectGroup = dynamic_cast<ObjectGroup*>(mapDocument()->currentLayer());
    if (!objectGroup)
        return;

    const ObjectTemplate *objectTemplate = readObjectTemplate(event->mimeData());
    if (!objectTemplate || !mapDocument()->templateAllowed(objectTemplate))
        return;

    MapObject *newMapObject = new MapObject;
    newMapObject->setObjectTemplate(objectTemplate);
    newMapObject->syncWithTemplate();
    newMapObject->setPosition(event->scenePos());

    auto addObjectCommand = new AddMapObjects(mapDocument(),
                                              objectGroup,
                                              newMapObject);

    mapDocument()->undoStack()->push(addObjectCommand);

    mapDocument()->setSelectedObjects(QList<MapObject*>() << newMapObject);
}

void MapScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
}

void MapScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
}

bool MapScene::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            Qt::KeyboardModifiers newModifiers = keyEvent->modifiers();

            if (mActiveTool && newModifiers != mCurrentModifiers) {
                mActiveTool->modifiersChanged(newModifiers);
                mCurrentModifiers = newModifiers;
            }
        }
        break;
    default:
        break;
    }

    return false;
}
