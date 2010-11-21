/*
 * objectselectiontool.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "objectselectiontool.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "movemapobject.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QPainter>
#include <QPalette>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

class SelectionRectangle : public QGraphicsItem
{
public:
    void setRectangle(const QRectF &rectangle);

    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

private:
    QRectF mRectangle;
};

} // namespace Internal
} // namespace Tiled

void SelectionRectangle::setRectangle(const QRectF &rectangle)
{
    prepareGeometryChange();
    mRectangle = rectangle;
}

QRectF SelectionRectangle::boundingRect() const
{
    return mRectangle.adjusted(-1, -1, 2, 2);
}

void SelectionRectangle::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *, QWidget *)
{
    if (mRectangle.isNull())
        return;

    // Draw a shadow
    QColor black(Qt::black);
    black.setAlpha(128);
    QPen pen(black, 2, Qt::DotLine);
    painter->setPen(pen);
    painter->drawRect(mRectangle.translated(1, 1));

    // Draw a rectangle in the highlight color
    QColor highlight = QApplication::palette().highlight().color();
    pen.setColor(highlight);
    highlight.setAlpha(32);
    painter->setPen(pen);
    painter->setBrush(highlight);
    painter->drawRect(mRectangle);
}


ObjectSelectionTool::ObjectSelectionTool(QObject *parent)
    : AbstractTool(tr("Select Objects"),
                   QIcon(QLatin1String(
                           ":images/22x22/tool-create-object.png")),
                   QKeySequence(tr("S")),
                   parent)
    , mSelectionRectangle(new SelectionRectangle)
    , mMode(NoMode)
{
    mSelectionRectangle->setZValue(10000);
}

ObjectSelectionTool::~ObjectSelectionTool()
{
    delete mSelectionRectangle;
}

void ObjectSelectionTool::activate(MapScene *scene)
{
    mMapScene = scene;
}

void ObjectSelectionTool::deactivate(MapScene *)
{
    mMapScene = 0;
}

void ObjectSelectionTool::mouseEntered()
{
}

void ObjectSelectionTool::mouseLeft()
{
}

void ObjectSelectionTool::mouseMoved(const QPointF &pos,
                                     Qt::KeyboardModifiers modifiers)
{
    if (mMode == Selecting) {
        updateSelection(pos);
    } else if (mMode == Moving) {
        MapRenderer *renderer = mapDocument()->renderer();
        QPointF diff = pos - mStart;

        if (modifiers & Qt::ControlModifier) {
            const QPointF alignPixelPos =
                    renderer->tileToPixelCoords(mAlignPosition);
            const QPointF newAlignPixelPos = alignPixelPos + diff;

            // Snap the position to the grid
            const QPointF newTileCoords =
                    renderer->pixelToTileCoords(newAlignPixelPos).toPoint();
            diff = renderer->tileToPixelCoords(newTileCoords) - alignPixelPos;
        }

        int i = 0;
        foreach (MapObjectItem *objectItem, mMovingItems) {
            const QPointF newPixelPos = mOldObjectItemPositions.at(i) + diff;
            const QPointF newPos = renderer->pixelToTileCoords(newPixelPos);
            objectItem->setPos(newPixelPos);
            objectItem->setZValue(newPixelPos.y());
            objectItem->mapObject()->setPosition(newPos);
            ++i;
        }
    }
}

void ObjectSelectionTool::mousePressed(const QPointF &pos,
                                       Qt::MouseButton button,
                                       Qt::KeyboardModifiers modifiers)
{
    if (button != Qt::LeftButton)
        return;

    mStart = pos;
    mOldSelection = mMapScene->selectedObjectItems();

    // If a map object item was pressed and no modifiers were held, enter
    // moving mode for this object
    if (modifiers == Qt::NoModifier) {
        foreach (QGraphicsItem *item, mMapScene->items(pos)) {
            MapObjectItem *objectItem = dynamic_cast<MapObjectItem*>(item);
            if (!objectItem)
                continue;

            if (!mMapScene->selectedObjectItems().contains(objectItem))
                updateSelection(pos);

            mMovingItems = mMapScene->selectedObjectItems();
            if (mMovingItems.isEmpty()) // Paranoia
                continue;

            mMode = Moving;

            // Remember the current object positions
            mOldObjectItemPositions.clear();
            mOldObjectPositions.clear();
            mAlignPosition = (*mMovingItems.begin())->mapObject()->position();

            foreach (MapObjectItem *objectItem, mMovingItems) {
                const QPointF &pos = objectItem->mapObject()->position();
                mOldObjectItemPositions += objectItem->pos();
                mOldObjectPositions += pos;
                if (pos.x() < mAlignPosition.x())
                    mAlignPosition.setX(pos.x());
                if (pos.y() < mAlignPosition.y())
                    mAlignPosition.setY(pos.y());
            }
            return;
        }
    }

    mMode = Selecting;
    updateSelection(pos);
    mMapScene->addItem(mSelectionRectangle);
}

void ObjectSelectionTool::mouseReleased(const QPointF &pos,
                                        Qt::MouseButton button)
{
    if (button != Qt::LeftButton)
        return;

    if (mMode == Selecting) {
        mMode = NoMode;
        mMapScene->removeItem(mSelectionRectangle);
    } else if (mMode == Moving) {
        mMode = NoMode;
        if (mStart == pos) // Move is a no-op
            return;

        QUndoStack *undoStack = mapDocument()->undoStack();
        undoStack->beginMacro(tr("Move Object(s)")); // TODO: plural
        int i = 0;
        foreach (MapObjectItem *objectItem, mMovingItems) {
            MapObject *object = objectItem->mapObject();
            const QPointF oldPos = mOldObjectPositions.at(i);
            undoStack->push(new MoveMapObject(mapDocument(), object, oldPos));
            ++i;
        }
        undoStack->endMacro();

        mOldObjectItemPositions.clear();
        mOldObjectPositions.clear();
        mMovingItems.clear();
    }
}

void ObjectSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;
}

void ObjectSelectionTool::languageChanged()
{
    setName(tr("Select Objects"));
    setShortcut(QKeySequence(tr("S")));
}

void ObjectSelectionTool::updateSelection(const QPointF &pos)
{
    QRectF rect = QRectF(mStart, pos).normalized();
    mSelectionRectangle->setRectangle(rect);

    // Make sure the rect has some contents, otherwise intersects returns false
    rect.setWidth(qMax(qreal(1), rect.width()));
    rect.setHeight(qMax(qreal(1), rect.height()));

    QSet<MapObjectItem*> selectedItems;

    foreach (QGraphicsItem *item, mMapScene->items(rect)) {
        MapObjectItem *mapObjectItem = dynamic_cast<MapObjectItem*>(item);
        if (mapObjectItem) {
            selectedItems.insert(mapObjectItem);
            if (mMode == NoMode)
                break;
        }
    }

    QSet<MapObjectItem*> newSelection;

    if (mModifiers == Qt::ControlModifier) {
        newSelection = mOldSelection - selectedItems;
    } else if (mModifiers == Qt::ShiftModifier) {
        newSelection = mOldSelection | selectedItems;
    } else if (mModifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
        newSelection = mOldSelection & selectedItems;
    } else {
        newSelection = selectedItems;
    }

    mMapScene->setSelectedObjectItems(newSelection);
}
