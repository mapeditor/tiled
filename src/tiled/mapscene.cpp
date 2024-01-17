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
#include "snaphelper.h"
#include "stylehelper.h"
#include "templatemanager.h"
#include "tilesetmanager.h"
#include "toolmanager.h"
#include "world.h"
#include "worldmanager.h"

#include <QApplication>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QPalette>

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
        connect(mMapDocument, &MapDocument::changed,
                this, &MapScene::changeEvent);
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
    for (auto mapItem : std::as_const(mMapItems))
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
 * Sets the painter scale on the MapRenderer instances for all MapItem
 * instances.
 */
void MapScene::setPainterScale(qreal painterScale)
{
    for (auto mapItem : std::as_const(mMapItems))
        mapItem->mapDocument()->renderer()->setPainterScale(painterScale);
}

void MapScene::setSuppressMouseMoveEvents(bool suppress)
{
    mSuppressMouseMoveEvents = suppress;

    if (!suppress && mMouseMoveEventSuppressed) {
        // Replay the last mouse move event
        toolMouseMoved(mLastMousePos, mLastModifiers);
        mMouseMoveEventSuppressed = false;
    }
}

/**
 * Returns the bounding rect of the map. This can be different from the
 * sceneRect() when multiple maps are displayed.
 */
QRectF MapScene::mapBoundingRect() const
{
    if (auto item = mapItem(mMapDocument))
        return item->boundingRect();
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

        if (!mSelectedTool)
            return; // Tool deactivated itself upon activation

        mToolModifiers = QApplication::keyboardModifiers();
        mSelectedTool->modifiersChanged(mToolModifiers);

        if (mUnderMouse) {
            mSelectedTool->mouseEntered();
            mSelectedTool->mouseMoved(mLastMousePos, mToolModifiers);
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

void MapScene::setOverrideBackgroundColor(QColor backgroundColor)
{
    if (mOverrideBackgroundColor == backgroundColor)
        return;

    mOverrideBackgroundColor = backgroundColor;
    updateBackgroundColor();
};

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

    QPointF viewCenter = mViewRect.center();

    Map *map = layer.map();
    if (const MapItem *mapItem = mMapItems.value(map))
        viewCenter -= mapItem->pos() + map->parallaxOrigin();

    const QPointF parallaxFactor = layer.effectiveParallaxFactor();
    return QPointF((1.0 - parallaxFactor.x()) * viewCenter.x(),
                   (1.0 - parallaxFactor.y()) * viewCenter.y());
}

/**
 * Refreshes the map scene.
 */
void MapScene::refreshScene()
{
    QHash<Map*, MapItem*> mapItems;

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

        for (const WorldMapEntry &mapEntry : contextMaps) {
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
                mapItems.insert(mapDocument->map(), mapItem);
            }
        }
    } else {
        auto mapItem = takeOrCreateMapItem(mMapDocument->sharedFromThis(), MapItem::Editable);
        mapItems.insert(mMapDocument->map(), mapItem);
    }

    mMapItems.swap(mapItems);
    qDeleteAll(mapItems);       // delete all map items that didn't get reused

    for (MapItem *mapItem : std::as_const(mMapItems))
        mapItem->updateLayerPositions();

    updateBackgroundColor();
    updateSceneRect();

    emit sceneRefreshed();
}

void MapScene::updateDefaultBackgroundColor()
{
    const QColor darkColor = QGuiApplication::palette().dark().color();
    if (mDefaultBackgroundColor != darkColor) {
        mDefaultBackgroundColor = darkColor;
        updateBackgroundColor();
    }
}

void MapScene::updateBackgroundColor()
{
    if (mOverrideBackgroundColor.isValid()) {
        setBackgroundBrush(mOverrideBackgroundColor);
        return;
    }

    if (mMapDocument) {
        const QColor &backgroundColor = mMapDocument->map()->backgroundColor();
        if (backgroundColor.isValid()) {
            setBackgroundBrush(backgroundColor);
            return;
        }
    }

    setBackgroundBrush(mDefaultBackgroundColor);
}

void MapScene::updateSceneRect()
{
    QRectF sceneRect;

    for (MapItem *mapItem : std::as_const(mMapItems))
        sceneRect |= mapItem->boundingRect().translated(mapItem->pos());

    setSceneRect(sceneRect);
}

void MapScene::setWorldsEnabled(bool enabled)
{
    if (mWorldsEnabled == enabled)
        return;

    mWorldsEnabled = enabled;

    for (MapItem *mapItem : std::as_const(mMapItems))
        mapItem->setVisible(mWorldsEnabled || mapItem->mapDocument() == mMapDocument);
}

MapItem *MapScene::takeOrCreateMapItem(const MapDocumentPtr &mapDocument, MapItem::DisplayMode displayMode)
{
    // Try to reuse an existing map item
    auto mapItem = mMapItems.take(mapDocument->map());
    if (!mapItem) {
        mapItem = new MapItem(mapDocument, displayMode);
        mapItem->setShowTileCollisionShapes(mShowTileCollisionShapes);
        connect(mapItem, &MapItem::boundingRectChanged, this, &MapScene::updateSceneRect);
        connect(this, &MapScene::parallaxParametersChanged, mapItem, &MapItem::updateLayerPositions);
        addItem(mapItem);
    } else {
        mapItem->setDisplayMode(displayMode);
    }
    return mapItem;
}

void MapScene::changeEvent(const ChangeEvent &change)
{
    switch (change.type) {
    case ChangeEvent::MapChanged:
        if (static_cast<const MapChangeEvent&>(change).property == Map::ParallaxOriginProperty)
            emit parallaxParametersChanged();
        break;
    case ChangeEvent::TilesetChanged:{
        auto &tilesetChange = static_cast<const TilesetChangeEvent&>(change);
        switch (tilesetChange.property) {
        case Tileset::FillModeProperty:
        case Tileset::TileRenderSizeProperty:
            repaintTileset(tilesetChange.tileset);
            break;
        }
        break;
    }
    default:
        break;
    }
}

/**
 * Updates the possibly changed background color.
 */
void MapScene::mapChanged()
{
    updateBackgroundColor();
}

void MapScene::repaintTileset(Tileset *tileset)
{
    for (MapItem *mapItem : std::as_const(mMapItems)) {
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    case QEvent::Leave:
#else
    case QEvent::GraphicsSceneLeave:
#endif
        mUnderMouse = false;
        if (mSelectedTool)
            mSelectedTool->mouseLeft();
        break;
    case QEvent::FontChange:
        emit fontChanged();
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
    mLastModifiers = mouseEvent->modifiers();

    if (mSuppressMouseMoveEvents) {
        mMouseMoveEventSuppressed = true;
        return;
    }
    if (!mMapDocument)
        return;

    QGraphicsScene::mouseMoveEvent(mouseEvent);

    // Currently we always want to inform the active tool about mouse move
    // events, regardless of whether this event was delived to a graphics item
    // as a hover event. This is due to the behavior of MapItem, which needs
    // to accept hover events but should not block them here.
//    if (mouseEvent->isAccepted())
//        return;

    if (toolMouseMoved(mLastMousePos, mLastModifiers))
        mouseEvent->accept();
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

    QPointF pos = event->scenePos();
    SnapHelper(mapDocument()->renderer(), event->modifiers()).snap(pos);

    MapObject *newMapObject = new MapObject;
    newMapObject->setObjectTemplate(objectTemplate);
    newMapObject->syncWithTemplate();
    newMapObject->setPosition(pos);

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
            const Qt::KeyboardModifiers newModifiers = keyEvent->modifiers();

            if (mSelectedTool && newModifiers != mToolModifiers) {
                mSelectedTool->modifiersChanged(newModifiers);
                mToolModifiers = newModifiers;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

bool MapScene::toolMouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (!mSelectedTool)
        return false;

    if (mToolModifiers != modifiers) {
        mToolModifiers = modifiers;
        mSelectedTool->modifiersChanged(modifiers);
    }

    mSelectedTool->mouseMoved(pos, modifiers);
    return true;
}

#include "moc_mapscene.cpp"
