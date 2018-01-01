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
#include "map.h"
#include "mapitem.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "preferences.h"
#include "templatemanager.h"
#include "stylehelper.h"
#include "toolmanager.h"
#include "tilesetmanager.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMapDocument(nullptr),
    mMapItem(nullptr),
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

    Preferences *prefs = Preferences::instance();
    connect(prefs, &Preferences::showGridChanged, this, &MapScene::setGridVisible);
    connect(prefs, &Preferences::gridColorChanged, this, [this] { update(); });

    mGridVisible = prefs->showGrid();

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
        connect(mMapDocument, &MapDocument::tileLayerChanged,
                this, &MapScene::tileLayerChanged);
        connect(mMapDocument, &MapDocument::layerChanged,
                this, &MapScene::layerChanged);
        connect(mMapDocument, &MapDocument::currentLayerChanged,
                this, &MapScene::currentLayerChanged);
        connect(mMapDocument, &MapDocument::tilesetTileOffsetChanged,
                this, &MapScene::adaptToTilesetTileSizeChanges);
        connect(mMapDocument, &MapDocument::tileImageSourceChanged,
                this, &MapScene::adaptToTileSizeChanges);
        connect(mMapDocument, &MapDocument::tilesetReplaced,
                this, &MapScene::tilesetReplaced);
    }

    refreshScene();
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
    clear();

    if (!mMapDocument) {
        setSceneRect(QRectF());
        return;
    }

    mMapItem = new MapItem(mMapDocument);
    addItem(mMapItem);

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
    QRectF sceneRect = mMapDocument->renderer()->mapBoundingRect();

    QMargins margins = mMapDocument->map()->computeLayerOffsetMargins();
    sceneRect.adjust(-margins.left(),
                     -margins.top(),
                     margins.right(),
                     margins.bottom());

    QMargins drawMargins = mMapDocument->map()->drawMargins();
    sceneRect.adjust(qMin(0, -drawMargins.left()),
                     qMin(0, -drawMargins.top()),
                     qMax(0, drawMargins.right()),
                     qMax(0, drawMargins.bottom()));

    setSceneRect(sceneRect);
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
    if (mCurrentModifiers != Qt::NoModifier)
        mActiveTool->modifiersChanged(mCurrentModifiers);

    if (mUnderMouse) {
        mActiveTool->mouseEntered();
        mActiveTool->mouseMoved(mLastMousePos, Qt::KeyboardModifiers());
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

void MapScene::currentLayerChanged()
{
    // New layer may have a different offset, affecting the grid
    if (mGridVisible)
        update();
}

/**
 * Adapts the scene, layers and objects to new map size, orientation or
 * background color.
 */
void MapScene::mapChanged()
{
    updateSceneRect();

    const Map *map = mMapDocument->map();
    if (map->backgroundColor().isValid())
        setBackgroundBrush(map->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);
}

void MapScene::repaintTileset(Tileset *tileset)
{
    if (!mMapDocument)
        return;

    if (contains(mMapDocument->map()->tilesets(), tileset))
        update();
}

void MapScene::tileLayerChanged(TileLayer *, MapDocument::TileLayerChangeFlags flags)
{
    if (flags & MapDocument::LayerBoundsChanged)
        updateSceneRect();
}

/**
 * A layer has changed. This can mean that the layer visibility, opacity or
 * offset changed.
 */
void MapScene::layerChanged(Layer *)
{
    // Layer offset may have changed, affecting the scene rect and grid
    updateSceneRect();
    if (mGridVisible)
        update();
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
 * Sets whether the tile grid is visible.
 */
void MapScene::setGridVisible(bool visible)
{
    if (mGridVisible == visible)
        return;

    mGridVisible = visible;
    update();
}

/**
 * QGraphicsScene::drawForeground override that draws the tile grid.
 */
void MapScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (!mMapDocument || !mGridVisible)
        return;

    QPointF offset;

    // Take into account the offset of the current layer
    if (Layer *layer = mMapDocument->currentLayer()) {
        offset = layer->totalOffset();
        painter->translate(offset);
    }

    Preferences *prefs = Preferences::instance();
    mMapDocument->renderer()->drawGrid(painter,
                                       rect.translated(-offset),
                                       prefs->gridColor());
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
    if (mouseEvent->isAccepted())
        return;

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

/**
 * Override to ignore drag enter events except for templates.
 */
void MapScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (!event->mimeData()->hasFormat(QLatin1String(TEMPLATES_MIMETYPE)))
        event->ignore();
}

/**
 * Accepts dropping a single template into an object group
 */
void MapScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    ObjectGroup *objectGroup = dynamic_cast<ObjectGroup*>(mapDocument()->currentLayer());
    if (!objectGroup || !mimeData->hasFormat(QLatin1String(TEMPLATES_MIMETYPE)))
        return;

    QByteArray encodedData = mimeData->data(QLatin1String(TEMPLATES_MIMETYPE));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    TemplateManager *templateManager = TemplateManager::instance();

    QString fileName;
    stream >> fileName;

    const ObjectTemplate *objectTemplate = templateManager->findObjectTemplate(fileName);
    if (!objectTemplate)
        return;

    MapObject *newMapObject = new MapObject();
    newMapObject->setObjectTemplate(objectTemplate);
    newMapObject->syncWithTemplate();
    newMapObject->setPosition(event->scenePos());

    auto addObjectCommand = new AddMapObject(mapDocument(),
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
