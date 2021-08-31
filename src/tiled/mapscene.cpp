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
#include "debugdrawitem.h"
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
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QPalette>

#include "qtcompat_p.h"

using namespace Tiled;

SessionOption<bool> MapScene::enableWorlds { "mapScene.enableWorlds", true };

MapScene::MapScene(QObject *parent)
    : QGraphicsScene(parent)
    , mWorldsEnabled(enableWorlds)
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

    mEnableWorldsCallback = enableWorlds.onChange([this] { setWorldsEnabled(enableWorlds); });

#ifdef QT_DEBUG
    mDebugDrawItem = new DebugDrawItem;
    addItem(mDebugDrawItem);
#endif
}

MapScene::~MapScene()
{
    enableWorlds.unregister(mEnableWorldsCallback);

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
        connect(mMapDocument, &MapDocument::tilesetTilePositioningChanged,
                this, [this] { update(); });
        connect(mMapDocument, &MapDocument::tileImageSourceChanged,
                this, [this] { update(); });
        connect(mMapDocument, &MapDocument::tilesetReplaced,
                this, &MapScene::tilesetReplaced);
    }

    refreshScene();
    emit mapDocumentChanged(mMapDocument);
}

void MapScene::setShowTileCollisionShapes(bool enabled)
{
    if (mShowTileCollisionShapes == enabled)
        return;

    mShowTileCollisionShapes = enabled;
    for (auto mapItem : qAsConst(mMapItems))
        mapItem->setShowTileCollisionShapes(enabled);
}

void MapScene::setParallaxEnabled(bool enabled)
{
    if (mParallaxEnabled == enabled)
        return;

    mParallaxEnabled = enabled;
    emit parallaxParametersChanged();
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
    if (mSelectedTool == tool)
        return;

    if (mSelectedTool) {
        if (mUnderMouse)
            mSelectedTool->mouseLeft();
        mSelectedTool->deactivate(this);
        mSelectedTool = nullptr;
    }

    if (tool && mMapDocument) {
        mSelectedTool = tool;
        mSelectedTool->activate(this);

        mCurrentModifiers = QApplication::keyboardModifiers();
        mSelectedTool->modifiersChanged(mCurrentModifiers);

        if (mUnderMouse) {
            mSelectedTool->mouseEntered();
            mSelectedTool->mouseMoved(mLastMousePos, mCurrentModifiers);
        }
    }
}

/**
 * Sets the area of the scene that is currently visible in the MapView.
 */
void MapScene::setViewRect(const QRectF &rect)
{
    if (mViewRect == rect)
        return;

    mViewRect = rect;

    if (mParallaxEnabled)
        emit parallaxParametersChanged();
}

/**
 * Returns the position the given layer is supposed to have, taking into
 * account its offset and the parallax factor along with the current view rect.
 */
QPointF MapScene::absolutePositionForLayer(const Layer &layer) const
{
    return layer.totalOffset() + parallaxOffset(layer);
}

/**
 * Returns the position for a layer item.
 *
 * Since layer items are in a hierarchy where translation of the parent layer
 * item affects the child layer items, the local offset is used instead of the
 * total offset.
 *
 * Similarly, since the effective parallax factor is applied, this factor is
 * ignored for group layers.
 */
QPointF MapScene::layerItemPosition(const Layer &layer) const
{
    return layer.offset() + (layer.isGroupLayer() ? QPointF()
                                                  : parallaxOffset(layer));
}

/**
 * Returns the parallax offset of the given layer, taking into account its
 * parallax factor in combination with the current view rect.
 */
QPointF MapScene::parallaxOffset(const Layer &layer) const
{
    if (!mParallaxEnabled)
        return {};

    const QPointF parallaxFactor = layer.effectiveParallaxFactor();
    const QPointF viewCenter = mViewRect.center();
    return QPointF((1.0 - parallaxFactor.x()) * viewCenter.x(),
                   (1.0 - parallaxFactor.y()) * viewCenter.y());
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

    const WorldManager &worldManager = WorldManager::instance();
    const QString currentMapFile = mMapDocument->canonicalFilePath();

    if (const World *world = worldManager.worldForMap(currentMapFile)) {
        const QPoint currentMapPosition = world->mapRect(currentMapFile).topLeft();
        auto const contextMaps = world->contextMaps(currentMapFile);

        for (const World::MapEntry &mapEntry : contextMaps) {
            MapDocumentPtr mapDocument;

            if (mapEntry.fileName == currentMapFile) {
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
                mapItem->setVisible(mWorldsEnabled || mapDocument == mMapDocument);
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

    emit sceneRefreshed();
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

void MapScene::setWorldsEnabled(bool enabled)
{
    if (mWorldsEnabled == enabled)
        return;

    mWorldsEnabled = enabled;

    for (MapItem *mapItem : qAsConst(mMapItems))
        mapItem->setVisible(mWorldsEnabled || mapItem->mapDocument() == mMapDocument);
}

MapItem *MapScene::takeOrCreateMapItem(const MapDocumentPtr &mapDocument, MapItem::DisplayMode displayMode)
{
    // Try to reuse an existing map item
    auto mapItem = mMapItems.take(mapDocument.data());
    if (!mapItem) {
        mapItem = new MapItem(mapDocument, displayMode);
        mapItem->setShowTileCollisionShapes(mShowTileCollisionShapes);
        connect(mapItem, &MapItem::boundingRectChanged, this, &MapScene::updateSceneRect);
        connect(this, &MapScene::parallaxParametersChanged, mapItem, &MapItem::updateLayerPositions);
        addItem(mapItem);
        mapItem->updateLayerPositions();
    } else {
        mapItem->setDisplayMode(displayMode);
    }
    return mapItem;
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

void MapScene::tilesetReplaced(int index, Tileset *tileset, Tileset *oldTileset)
{
    Q_UNUSED(index)
    Q_UNUSED(oldTileset)

    repaintTileset(tileset);
}

/**
 * Override for handling enter and leave events.
 */
bool MapScene::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        mUnderMouse = true;
        if (mSelectedTool)
            mSelectedTool->mouseEntered();
        break;
    case QEvent::Leave:
        mUnderMouse = false;
        if (mSelectedTool)
            mSelectedTool->mouseLeft();
        break;
    default:
        break;
    }

    return QGraphicsScene::event(event);
}

void MapScene::keyPressEvent(QKeyEvent *event)
{
    if (mSelectedTool)
        mSelectedTool->keyPressed(event);

    if (!(mSelectedTool && event->isAccepted()))
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

    if (mSelectedTool) {
        mSelectedTool->mouseMoved(mouseEvent->scenePos(),
                                  mouseEvent->modifiers());
        mouseEvent->accept();
    }
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mousePressEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mSelectedTool) {
        mouseEvent->accept();
        mSelectedTool->mousePressed(mouseEvent);
    }
}

void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mSelectedTool) {
        mouseEvent->accept();
        mSelectedTool->mouseReleased(mouseEvent);
    }
}

void MapScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mSelectedTool) {
        mouseEvent->accept();
        mSelectedTool->mouseDoubleClicked(mouseEvent);
    }
}

static const ObjectTemplate *readObjectTemplate(const QMimeData *mimeData)
{
    const auto urls = mimeData->urls();
    if (urls.size() != 1)
        return nullptr;

    const QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return nullptr;

    const QFileInfo info(fileName);
    if (info.isDir())
        return nullptr;

    auto objectTemplate = TemplateManager::instance()->loadObjectTemplate(info.absoluteFilePath());
    return objectTemplate->object() ? objectTemplate : nullptr;
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

    mapDocument()->setSelectedObjects({newMapObject});
}

void MapScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)
}

void MapScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)
}

bool MapScene::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            Qt::KeyboardModifiers newModifiers = keyEvent->modifiers();

            if (mSelectedTool && newModifiers != mCurrentModifiers) {
                mSelectedTool->modifiersChanged(newModifiers);
                mCurrentModifiers = newModifiers;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

#include "moc_mapscene.cpp"
