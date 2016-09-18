/*
 * rtbselectareatool.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbselectareatool.h"

#include "brushitem.h"
#include "mapobjectitem.h"
#include "mapscene.h"
#include "snaphelper.h"
#include "movemapobject.h"
#include "mapobjectmodel.h"
#include "clipboardmanager.h"
#include "erasetiles.h"
#include "painttilelayer.h"
#include "geometry.h"
#include "objectgroup.h"
#include "tilesetmanager.h"
#include "addremovetileset.h"
#include "addremovemapobject.h"

#include "rtbmapsettings.h"
#include "rtbinserttool.h"

#include <QGraphicsItem>

using namespace Tiled;
using namespace Tiled::Internal;

/**
 * Graphical item to show the move curser
 */
class SelectedAreaItem : public QGraphicsItem
{
public:
    SelectedAreaItem(QRectF boundingRect, QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
        , mRect(boundingRect)
    {
        setCursor(Qt::SizeAllCursor);
    }

    QRectF boundingRect() const{ return mRect; }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

private:
    QRectF mRect;
};

void SelectedAreaItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

//===================================================================================================

RTBSelectAreaTool::RTBSelectAreaTool(QObject *parent)
    : TileSelectionTool(parent)
    , mIsActive(false)
    , mAction(NoAction)
    , mMapScene(0)
    , mMousePressed(false)
    , mBrushBehavior(Free)
    , mTileX(0), mTileY(0)
    , mEditedRegion(0)
{
    languageChanged();
}

RTBSelectAreaTool::~RTBSelectAreaTool()
{

}

void RTBSelectAreaTool::tilePositionChanged(const QPoint &tilePos)
{
    if (mBrushBehavior == Paint)
    {
        // Draw a line from the previous point to avoid gaps, skipping the
        // first point, since it was painted when the mouse was pressed, or the
        // last time the mouse was moved.
        QVector<QPoint> points = pointsOnLine(mPrevTilePosition, tilePos);
        QRegion editedRegion;

        for (int i = 1; i < points.size(); ++i)
        {
            drawPreviewLayer(QVector<QPoint>() << points.at(i));

            // Only update the brush item for the last drawn piece
            if (i == points.size() - 1)
                brushItem()->setTileLayer(mPreviewLayer);

            editedRegion |= doPaint(Mergeable | SuppressRegionEdited);
        }

        mapDocument()->emitRegionEdited(editedRegion, currentTileLayer());
    }
    else if(mAction == Moving)
        updatePreview();
    else
        TileSelectionTool::tilePositionChanged(tilePos);


    mPrevTilePosition = tilePos;
}

void RTBSelectAreaTool::languageChanged()
{
    setName(tr("Select Area"));
    setShortcut(QKeySequence(tr("A")));
}

void RTBSelectAreaTool::mapDocumentChanged(MapDocument *oldDocument,
                                    MapDocument *newDocument)
{
    TileSelectionTool::mapDocumentChanged(oldDocument, newDocument);

    // Reset the brush, since it probably became invalid
    brushItem()->setTileRegion(QRegion());
    setStamp(TileStamp());
}

void RTBSelectAreaTool::setStamp(const TileStamp &stamp)
{
    if (mStamp == stamp)
        return;

    mStamp = stamp;

    updatePreview();
}

void RTBSelectAreaTool::updateEnabledState()
{
    setEnabled(TileSelectionTool::mapDocument());
}

void RTBSelectAreaTool::activate(MapScene *scene)
{
    mIsActive = true;
    mMapScene = scene;
    mapDocument()->setCurrentLayerIndex(RTBMapSettings::FloorID);
    TileSelectionTool::activate(scene);
}

void RTBSelectAreaTool::deactivate(MapScene *scene)
{
    removeSelectedAreaItems();

    // clear selection
    mapDocument()->setSelectedArea(QRegion());
    QSet<MapObjectItem*> selectedItems;
    mMapScene->setSelectedObjectItems(selectedItems);

    mIsActive = false;
    mMapScene = 0;
    mEditedRegion = 0;
    TileSelectionTool::deactivate(scene);
}

void RTBSelectAreaTool::deleteArea()
{
    TileLayer *tileLayer = mapDocument()->currentLayer()->asTileLayer();
    const QRegion &selectedArea = mapDocument()->selectedArea();
    const QList<MapObject*> &selectedObjects = mapDocument()->selectedObjects();

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Delete"));
    // delete tiles
    undoStack->push(new EraseTiles(mapDocument(), tileLayer, selectedArea));

    // delete objects
    if(!selectedObjects.isEmpty())
    {
        foreach (MapObject *mapObject, selectedObjects)
            undoStack->push(new RemoveMapObject(mapDocument(), mapObject));
    }

    mapDocument()->setSelectedArea(QRegion());
    QSet<MapObjectItem*> selectedItems;
    mMapScene->setSelectedObjectItems(selectedItems);
    removeSelectedAreaItems();

    undoStack->endMacro();
}

void RTBSelectAreaTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mMousePressed = true;

        const MapRenderer *renderer = mapDocument()->renderer();

        QPointF tileStart = renderer->screenToTileCoords(event->scenePos());
        tileStart = QPointF((int) std::floor(tileStart.x()), (int) std::floor(tileStart.y()));
        mStart = renderer->tileToScreenCoords(tileStart);

        QRectF selectedArea = mapDocument()->selectedArea().boundingRect();
        QPointF tilePos = renderer->screenToTileCoords(mStart);
        tilePos = QPointF(std::floor(tilePos.x()), std::floor(tilePos.y()));
        mDragDelta = tilePos - selectedArea.topLeft();
    }    

    if(mAction == NoAction || mAction == Selecting)
    {
        removeSelectedAreaItems();
        TileSelectionTool::mousePressed(event);
    }
}

void RTBSelectAreaTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (!brushItem()->isVisible())
        return;

    switch (mAction) {
    case NoAction:
        TileSelectionTool::mouseReleased(event);
        // deselect objects
        mapDocument()->setSelectedObjects(QList<MapObject*>());
        mEditedRegion = 0;

        break;
    case Selecting:
        TileSelectionTool::mouseReleased(event);
        updateSelection(event->modifiers());
        mAction = NoAction;
        mEditedRegion = 0;

        break;
    case Moving:
    {
        beginPaint();
        finishMoving(event->scenePos());
        TileSelectionTool::mouseReleased(event);

        // set moving false to activate drawing (LBs)
        foreach (MovingObject movingObject, mMovingObjects)
            movingObject.item->setIsMoving(false);

        mMovingObjects.clear();
        mAction = NoAction;
        mBrushBehavior = Free;
        brushItem()->clear();

        // select area which was moved
        setSelectedArea(mPreviewLayer.data()->region());
        const MapRenderer *renderer = mapDocument()->renderer();
        for(QRectF rect : mapDocument()->selectedArea().rects())
        {
            SelectedAreaItem *item = new SelectedAreaItem(QRectF(renderer->tileToScreenCoords(rect.topLeft())
                                                         , renderer->tileToScreenCoords(rect.bottomRight())));
            mMapScene->addItem(item);
            mSelectedAreaItems.append(item);
        }

        mapDocument()->undoStack()->endMacro();
        break;
    }
    }

    mMousePressed = false;
}

void RTBSelectAreaTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    // not moving if one of the modifiers is active
    bool isModifierActive = false;
    if(modifiers == Qt::ControlModifier || modifiers == Qt::ShiftModifier
            || modifiers == (Qt::ControlModifier | Qt::ShiftModifier))
    {
        isModifierActive = true;
    }

    if(mAction == NoAction && mMousePressed && !isModifierActive)
    {
        const MapRenderer *renderer = mapDocument()->renderer();
        QPointF tilePos = renderer->screenToTileCoords(pos);
        QRectF selectedArea = mapDocument()->selectedArea().boundingRect();

        if(selectedArea.contains(tilePos))
        {
            mapDocument()->undoStack()->beginMacro(tr("Move Area"));
            startMoving(modifiers);
        }
        else
            mAction = Selecting;
    }
    else if(isModifierActive && mMousePressed)
    {
        mAction = Selecting;
    }

    updateMousePosition(pos);

    switch (mAction)
    {
        case NoAction:
        case Selecting:
            TileSelectionTool::mouseMoved(pos, modifiers);
            break;
        case Moving:
            // do it in the updateMousePosition function so is only updated if the cell is changed
            //updateMovingItems(pos, modifiers);
            break;
    }
}

void RTBSelectAreaTool::updateMousePosition(const QPointF &pos)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF tilePosF = renderer->screenToTileCoords(pos);
    QPoint tilePos((int) std::floor(tilePosF.x()),
                     (int) std::floor(tilePosF.y()));

    if (mTileX != tilePos.x() || mTileY != tilePos.y()) {
        mTileX = tilePos.x();
        mTileY = tilePos.y();

        tilePositionChanged(tilePos);
        updateStatusInfo();

        if(mAction == Moving)
            updateMovingItems(pos, Qt::NoModifier);
    }
}

void RTBSelectAreaTool::updateStatusInfo()
{
    Tile *tile = 0;

    if (const TileLayer *tileLayer = currentTileLayer()) {
        const QPoint pos = tilePosition() - tileLayer->position();
        if (tileLayer->contains(pos))
            tile = tileLayer->cellAt(pos).tile;
    }

    QString tileIdString = tile ? QString::number(tile->id()) : tr("empty");
    setStatusInfo(QString(QLatin1String("%1, %2 [%3]"))
                  .arg(mTileX).arg(mTileY).arg(tileIdString));
}

void RTBSelectAreaTool::updateSelection(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    const QRegion &selectedArea = mapDocument()->selectedArea();
    QSet<MapObjectItem*> selectedItems;

    foreach (QRectF rect, selectedArea.rects())
    {
        const MapRenderer *renderer = mapDocument()->renderer();
        QRectF newRect(renderer->tileToPixelCoords(rect.topLeft())
                       , renderer->tileToPixelCoords(rect.bottomRight()));

        QList<ObjectGroup*> objectGroups = mapDocument()->map()->objectGroups();
        QList<MapObject*> objects = objectGroups.at(0)->objects();
        objects.append(objectGroups.at(1)->objects());

        foreach (MapObject *mapObject, objects)
        {
            if(newRect.contains(mapObject->boundsUseTile().center()))
                selectedItems.insert(mMapScene->itemForObject(mapObject));
        }

        SelectedAreaItem *item = new SelectedAreaItem(QRectF(renderer->tileToScreenCoords(rect.topLeft())
                                                     , renderer->tileToScreenCoords(rect.bottomRight())));
        mMapScene->addItem(item);
        mSelectedAreaItems.append(item);
    }

    mMapScene->setSelectedObjectItems(selectedItems);
}

void RTBSelectAreaTool::startMoving(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    saveSelectionState();

    mAction = Moving;

    // set moving true to deactivate drawing (LBs)
    foreach (MovingObject movingObject, mMovingObjects)
    {
        movingObject.item->setIsMoving(true);
    }

    const MapRenderer *renderer = mapDocument()->renderer();
    mAlignPosition = renderer->tileToScreenCoords(mapDocument()->selectedArea().boundingRect().topLeft());

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    clipboardManager->copySelection(mapDocument());

    QScopedPointer<Map> map(clipboardManager->map());

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReferences(map->tilesets());
    mapDocument()->unifyTilesets(map.data());

    Map *stamp = map.take(); // TileStamp will take ownership
    setStamp(TileStamp(stamp));
    tilesetManager->removeReferences(stamp->tilesets());

    // delete map in clipboard
    clipboardManager->clear();

    TileLayer *tileLayer = mapDocument()->currentLayer()->asTileLayer();
    const QRegion &selectedArea = mapDocument()->selectedArea();

    QUndoStack *stack = mapDocument()->undoStack();
    stack->push(new EraseTiles(mapDocument(), tileLayer, selectedArea));

    mapDocument()->setSelectedArea(QRegion());

    // paint the stored region if the selected area move again
    if(mEditedRegion != 0 && !mEditedRegion->isEmpty())
    {
        MapRenderer *renderer = mapDocument()->renderer();
        QPointF insertPos = snapToGrid(mStart, Qt::NoModifier);
        insertPos = renderer->screenToTileCoords(insertPos);
        insertPos = QPointF(std::floor(insertPos.x()), std::floor(insertPos.y()));

        PaintTileLayer *paintEditedRegion = new PaintTileLayer(mapDocument(),
                                                   tileLayer,
                                                   insertPos.x() - mDragDelta.x(),
                                                   insertPos.y() - mDragDelta.y(),
                                                   mEditedRegion);

        mapDocument()->undoStack()->push(paintEditedRegion);
        mapDocument()->emitRegionEdited(mEditedRegion->region(), tileLayer);
    }
}

void RTBSelectAreaTool::updateMovingItems(const QPointF &pos,
                                            Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();

    // convert screen pos to int tile pos
    QPointF tilePos = renderer->screenToTileCoords(pos);
    tilePos = QPointF((int) std::floor(tilePos.x()), (int) std::floor(tilePos.y()));

    QPointF diff = snapToGrid(renderer->tileToScreenCoords(tilePos) - mStart
                              , modifiers);

    foreach (const MovingObject &object, mMovingObjects)
    {
        const QPointF newPixelPos = object.oldItemPosition + diff;
        const QPointF newPos = renderer->screenToPixelCoords(newPixelPos);

        QPointF newTilePos = renderer->pixelToTileCoords(newPos);
        newTilePos.setY(newTilePos.y() - 1);

        // only move if the target is inside the map
        if(newTilePos.x() >= 0 && newTilePos.x() <= mapDocument()->map()->width()-1
                && newTilePos.y() >= 0 && newTilePos.y() <= mapDocument()->map()->height() - 1)
        {
            MapObject *mapObject = object.item->mapObject();
            mapObject->setPosition(newPos);
        }
    }

    mapDocument()->mapObjectModel()->emitObjectsChanged(changingObjects());
}

void RTBSelectAreaTool::finishMoving(const QPointF &pos)
{
    Q_ASSERT(mAction == Moving);
    mAction = NoAction;

    QPointF posGridPos = snapToGrid(pos, Qt::NoModifier);
    QPointF startGridPos = snapToGrid(mStart, Qt::NoModifier);
    // return if the new pos is the equal the old or the pos is outside the map
    if(startGridPos.x() == posGridPos.x() && startGridPos.y() == posGridPos.y()
            || posGridPos.x() < 0 || posGridPos.y() < 0
            || posGridPos.x() >= mapDocument()->map()->width() * 32 ||posGridPos.y() >= mapDocument()->map()->height() * 32)
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    foreach (const MovingObject &object, mMovingObjects) {
        undoStack->push(new MoveMapObject(mapDocument(),
                                          object.item->mapObject(),
                                          object.oldPosition));
    }

    mapDocument()->mapObjectModel()->emitObjectsChanged(changingObjects());
}

const QPointF RTBSelectAreaTool::snapToGrid(const QPointF &diff,
                                              Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();
    SnapHelper snapHelper(renderer, modifiers);

    if (snapHelper.snaps())
    {
        const QPointF alignScreenPos = renderer->pixelToScreenCoords(mAlignPosition);
        const QPointF newAlignScreenPos = alignScreenPos + diff;

        QPointF newAlignPixelPos = renderer->screenToPixelCoords(newAlignScreenPos);
        snapHelper.snap(newAlignPixelPos, mMapScene->selectedObjectItems(), false);

        return renderer->pixelToScreenCoords(newAlignPixelPos) - alignScreenPos;
    }

    return diff;
}

void RTBSelectAreaTool::saveSelectionState()
{
    mMovingObjects.clear();

    // Remember the initial state before moving, resizing or rotating
    foreach (MapObjectItem *item, mMapScene->selectedObjectItems())
    {
        MapObject *mapObject = item->mapObject();
        MovingObject object = {
            item,
            item->pos(),
            mapObject->position(),
            mapObject->size(),
            mapObject->polygon(),
            mapObject->rotation()
        };
        mMovingObjects.append(object);
    }
}

QList<MapObject *> RTBSelectAreaTool::changingObjects() const
{
    QList<MapObject*> changingObjects;
    changingObjects.reserve(mMovingObjects.size());

    foreach (const MovingObject &movingObject, mMovingObjects)
        changingObjects.append(movingObject.item->mapObject());

    return changingObjects;
}

/**
 * Updates the position of the brush item based on the mouse position.
 */
void RTBSelectAreaTool::updatePreview()
{
    if(mMousePressed)
        updatePreview(tilePosition());
}

void RTBSelectAreaTool::updatePreview(QPoint tilePos)
{
    QRegion tileRegion;

    if (mStamp.isEmpty())
    {
        mPreviewLayer.clear();
        tileRegion = QRect(tilePos, QSize(1, 1));
    }
    else
        drawPreviewLayer(QVector<QPoint>() << tilePos);

    brushItem()->setTileLayer(mPreviewLayer);
    if (!tileRegion.isEmpty())
        brushItem()->setTileRegion(tileRegion);
}

void RTBSelectAreaTool::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint();
}

QRegion RTBSelectAreaTool::doPaint(int flags)
{
    TileLayer *preview = mPreviewLayer.data();
    if (!preview)
        return QRegion();

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = mapDocument()->currentLayer()->asTileLayer();
    Q_ASSERT(tileLayer);

    // store the edited region
    const QRegion &selectedArea = preview->region();
    mEditedRegion = tileLayer->copy(selectedArea.translated(-tileLayer->x(),
                                                         -tileLayer->y()));

    if (!tileLayer->bounds().intersects(QRect(preview->x(),
                                              preview->y(),
                                              preview->width(),
                                              preview->height())))
        return QRegion();

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(),
                                               tileLayer,
                                               preview->x(),
                                               preview->y(),
                                               preview);

    if (!mMissingTilesets.isEmpty()) {
        for (const SharedTileset &tileset : mMissingTilesets)
            new AddTileset(mapDocument(), tileset, paint);

        mMissingTilesets.clear();
    }

    paint->setMergeable(flags & Mergeable);
    mapDocument()->undoStack()->push(paint);

    QRegion editedRegion = preview->region();
    if (! (flags & SuppressRegionEdited))
        mapDocument()->emitRegionEdited(editedRegion, tileLayer);

    return editedRegion;
}

struct PaintOperation {
    QPoint pos;
    TileLayer *stamp;
};

void RTBSelectAreaTool::drawPreviewLayer(const QVector<QPoint> &list)
{
    mPreviewLayer.clear();

    if (mStamp.isEmpty())
        return;

    mMissingTilesets.clear();
    QRegion paintedRegion;
    QVector<PaintOperation> operations;
    QHash<TileLayer *, QRegion> regionCache;

    for (const QPoint &p : list)
    {
        Map *variation = mStamp.randomVariation();
        mapDocument()->unifyTilesets(variation, mMissingTilesets);

        TileLayer *stamp = variation->layerAt(RTBMapSettings::FloorID)->asTileLayer();

        QRegion stampRegion;
        if (regionCache.contains(stamp)) {
            stampRegion = regionCache.value(stamp);
        } else {
            stampRegion = stamp->region();
            regionCache.insert(stamp, stampRegion);
        }

        // stamp position in relation to the mouse, like the objects
        QPoint startPos(p.x() - (mDragDelta.x()),
                        (p.y() - mDragDelta.y()));

        const QRegion region = stampRegion.translated(startPos.x(),
                                                      startPos.y());

        if (!paintedRegion.intersects(region)) {
            paintedRegion += region;

            PaintOperation op = { startPos, stamp };
            operations.append(op);
        }

        QRect bounds = paintedRegion.boundingRect();
        SharedTileLayer preview(new TileLayer(QString(),
                                              bounds.x(), bounds.y(),
                                              bounds.width(), bounds.height()));

        for (const PaintOperation &op : operations)
            preview->merge(op.pos - bounds.topLeft(), op.stamp);

        mPreviewLayer = preview;
    }
}

void RTBSelectAreaTool::removeSelectedAreaItems()
{
    foreach (SelectedAreaItem *item, mSelectedAreaItems)
         mMapScene->removeItem(item);

    mSelectedAreaItems.clear();
}
