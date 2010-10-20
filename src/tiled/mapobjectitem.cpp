/*
 * mapobjectitem.cpp
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "mapobjectitem.h"

#include "addremovemapobject.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "movemapobject.h"
#include "movemapobjecttogroup.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "objectpropertiesdialog.h"
#include "resizemapobject.h"
#include "utils.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

/**
 * A resize handle that allows resizing of a map object.
 */
class ResizeHandle : public QGraphicsItem
{
public:
    ResizeHandle(MapObjectItem *mapObjectItem);

    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    MapObjectItem *mMapObjectItem;
    QSizeF mOldSize;
};

} // namespace Internal
} // namespace Tiled


ResizeHandle::ResizeHandle(MapObjectItem *mapObjectItem)
    : QGraphicsItem(mapObjectItem)
    , mMapObjectItem(mapObjectItem)
{
    setCursor(Qt::SizeFDiagCursor);
    setFlag(QGraphicsItem::ItemIsMovable);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
#endif
}

QRectF ResizeHandle::boundingRect() const
{
    return QRectF(-5, -5, 10 + 1, 10 + 1);
}

void ResizeHandle::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *)
{
    painter->setBrush(mMapObjectItem->color());
    painter->setPen(Qt::black);
    painter->drawRect(QRectF(-5, -5, 10, 10));
}

void ResizeHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Remember the old size since we may resize the object
    if (event->button() == Qt::LeftButton)
        mOldSize = mMapObjectItem->mapObject()->size();

    QGraphicsItem::mousePressEvent(event);
}

void ResizeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);

    // If we resized the object, create an undo command
    MapObject *obj = mMapObjectItem->mapObject();
    if (event->button() == Qt::LeftButton && mOldSize != obj->size()) {
        MapDocument *document = mMapObjectItem->mapDocument();
        QUndoCommand *cmd = new ResizeMapObject(document, obj, mOldSize);
        document->undoStack()->push(cmd);
    }
}

QVariant ResizeHandle::itemChange(GraphicsItemChange change,
                                  const QVariant &value)
{
    if (!mMapObjectItem->mSyncing) {
        MapRenderer *renderer = mMapObjectItem->mapDocument()->renderer();

        if (change == ItemPositionChange) {
            // Calculate the absolute pixel position
            const QPointF itemPos = mMapObjectItem->pos();
            QPointF pixelPos = value.toPointF() + itemPos;

            // Calculate the new coordinates in tiles
            QPointF tileCoords = renderer->pixelToTileCoords(pixelPos);
            const QPointF objectPos = mMapObjectItem->mapObject()->position();
            tileCoords -= objectPos;
            tileCoords.setX(qMax(tileCoords.x(), qreal(0)));
            tileCoords.setY(qMax(tileCoords.y(), qreal(0)));
            if (QApplication::keyboardModifiers() & Qt::ControlModifier)
                tileCoords = tileCoords.toPoint();
            tileCoords += objectPos;

            return renderer->tileToPixelCoords(tileCoords) - itemPos;
        }
        else if (change == ItemPositionHasChanged) {
            // Update the size of the map object
            const QPointF newPos = value.toPointF() + mMapObjectItem->pos();
            QPointF tileCoords = renderer->pixelToTileCoords(newPos);
            tileCoords -= mMapObjectItem->mapObject()->position();
            mMapObjectItem->resize(QSizeF(tileCoords.x(), tileCoords.y()));
        }
    }

    return QGraphicsItem::itemChange(change, value);
}


MapObjectItem::MapObjectItem(MapObject *object, MapDocument *mapDocument,
                             ObjectGroupItem *parent):
    QGraphicsItem(parent),
    mObject(object),
    mMapDocument(mapDocument),
    mIsEditable(false),
    mSyncing(false),
    mResizeHandle(new ResizeHandle(this))
{
    syncWithMapObject();
    mResizeHandle->setVisible(false);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
#endif

    if (parent)
        setEditable(parent->isEditable());
}

void MapObjectItem::syncWithMapObject()
{
    // Update the whole object when the name or type has changed
    if (mObject->name() != mName || mObject->type() != mType) {
        mName = mObject->name();
        mType = mObject->type();
        update();
        mResizeHandle->update();
    }

    QString toolTip = mName;
    if (!mType.isEmpty())
        toolTip += QLatin1String(" (") + mType + QLatin1String(")");
    setToolTip(toolTip);

    MapRenderer *renderer = mMapDocument->renderer();
    const QPointF pixelPos = renderer->tileToPixelCoords(mObject->position());
    QRectF bounds = renderer->boundingRect(mObject);
    bounds.translate(-pixelPos);

    mSyncing = true;
    setPos(pixelPos);

    if (mBoundingRect != bounds) {
        // Notify the graphics scene about the geometry change in advance
        prepareGeometryChange();
        mBoundingRect = bounds;
        const QPointF bottomRight = mObject->bounds().bottomRight();
        const QPointF handlePos = renderer->tileToPixelCoords(bottomRight);
        mResizeHandle->setPos(handlePos - pixelPos);
    }
    mSyncing = false;
}

void MapObjectItem::setEditable(bool editable)
{
    if (editable == mIsEditable)
        return;

    mIsEditable = editable;

    setFlag(QGraphicsItem::ItemIsMovable, mIsEditable);
    mResizeHandle->setVisible(mIsEditable && !mObject->tile());
    if (mIsEditable)
        setCursor(Qt::SizeAllCursor);
    else
        unsetCursor();
}

QRectF MapObjectItem::boundingRect() const
{
    return mBoundingRect;
}

QPainterPath MapObjectItem::shape() const
{
    QPainterPath path = mMapDocument->renderer()->shape(mObject);
#if QT_VERSION >= 0x040600
    path.translate(-pos());
#else
    const QPointF p = pos();
    const int elementCount = path.elementCount();
    for (int i = 0; i < elementCount; i++) {
        const QPainterPath::Element &element = path.elementAt(i);
        path.setElementPositionAt(i, element.x - p.x(), element.y - p.y());
    }
#endif
    return path;
}

void MapObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *)
{
    painter->translate(-pos());
    const QColor color = MapObjectItem::color();
    mMapDocument->renderer()->drawMapObject(painter, mObject, color);
}

/**
 * Shows the context menu for map objects. The menu allows you to duplicate and
 * remove the map object, or do edit its properties.
 */
void MapObjectItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!mIsEditable)
        return;

    QMenu menu;
    QIcon dupIcon(QLatin1String(":images/16x16/stock-duplicate-16.png"));
    QIcon delIcon(QLatin1String(":images/16x16/edit-delete.png"));
    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));
    QAction *dupAction = menu.addAction(dupIcon, tr("&Duplicate Object"));
    QAction *removeAction = menu.addAction(delIcon, tr("&Remove Object"));
    menu.addSeparator();
    QMenu *moveToLayerMenu = menu.addMenu(tr("&Move To Layer"));
    menu.addSeparator();
    QAction *propertiesAction = menu.addAction(propIcon,
                                               tr("Object &Properties..."));

    // Fill out moveToLayerMenu
    typedef QMap<QAction*, ObjectGroup*> MoveToLayerActionMap;
    MoveToLayerActionMap moveToLayerActions;
    foreach (Layer *layer, mMapDocument->map()->layers()) {
        ObjectGroup *objectGroup = layer->asObjectGroup();
        if (!objectGroup || objectGroup == mObject->objectGroup())
            continue;

        QAction *action = moveToLayerMenu->addAction(objectGroup->name());
        moveToLayerActions.insert(action, objectGroup);
    }

    if (moveToLayerMenu->isEmpty()) {
        QAction *action =
                moveToLayerMenu->addAction(tr("No other object layers"));
        action->setEnabled(false);
    }

    Utils::setThemeIcon(removeAction, "edit-delete");
    Utils::setThemeIcon(propertiesAction, "document-properties");

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == dupAction) {
        MapDocument *doc = mMapDocument;
        doc->undoStack()->push(new AddMapObject(doc,
                                                mObject->objectGroup(),
                                                mObject->clone()));
    }
    else if (selectedAction == removeAction) {
        MapDocument *doc = mMapDocument;
        doc->undoStack()->push(new RemoveMapObject(doc, mObject));
    }
    else if (selectedAction == propertiesAction) {
        ObjectPropertiesDialog propertiesDialog(mMapDocument, mObject,
                                                event->widget());
        propertiesDialog.exec();
    }

    MoveToLayerActionMap::const_iterator i =
            moveToLayerActions.find(selectedAction);

    if (i != moveToLayerActions.end()) {
        MapDocument *doc = mMapDocument;
        doc->undoStack()->push(new MoveMapObjectToGroup(doc,
                                                        mObject,
                                                        i.value()));
    }
}

void MapObjectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Remember the old position since we may get moved
    if (event->button() == Qt::LeftButton) {
        mOldObjectPos = mObject->position();
        mOldItemPos = pos();
    }

    QGraphicsItem::mousePressEvent(event);
}

void MapObjectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);

    // If we got moved, create an undo command
    if (event->button() == Qt::LeftButton
        && mOldObjectPos != mObject->position()) {

        MapDocument *document = mMapDocument;
        QUndoCommand *cmd = new MoveMapObject(document, mObject, mOldObjectPos);
        document->undoStack()->push(cmd);
    }
}

QVariant MapObjectItem::itemChange(GraphicsItemChange change,
                                   const QVariant &value)
{
    if (!mSyncing) {
        MapRenderer *renderer = mMapDocument->renderer();

        if (change == ItemPositionChange
            && (QApplication::keyboardModifiers() & Qt::ControlModifier))
        {
            const QPointF pixelDiff = value.toPointF() - mOldItemPos;
            const QPointF newPixelPos =
                    renderer->tileToPixelCoords(mOldObjectPos) + pixelDiff;
            // Snap the position to the grid
            const QPointF newTileCoords =
                    renderer->pixelToTileCoords(newPixelPos).toPoint();

            return renderer->tileToPixelCoords(newTileCoords);
        }
        else if (change == ItemPositionHasChanged) {
            // Update the position of the map object
            const QPointF pixelDiff = value.toPointF() - mOldItemPos;
            const QPointF newPixelPos =
                    renderer->tileToPixelCoords(mOldObjectPos) + pixelDiff;
            mObject->setPosition(renderer->pixelToTileCoords(newPixelPos));
        }
    }

    return QGraphicsItem::itemChange(change, value);
}

void MapObjectItem::resize(const QSizeF &size)
{
    prepareGeometryChange();
    mObject->setSize(size);
    syncWithMapObject();
}

MapDocument *MapObjectItem::mapDocument() const
{
    return mMapDocument;
}

QColor MapObjectItem::color() const
{
    // Type color takes precedence
    const Qt::GlobalColor typeColor = colorForType();
    if (typeColor != Qt::transparent)
        return typeColor;

    // Get color from object group
    const ObjectGroup *objectGroup = mObject->objectGroup();
    if (objectGroup && objectGroup->color().isValid())
        return objectGroup->color();

    // Fallback color
    return Qt::gray;
}

Qt::GlobalColor MapObjectItem::colorForType() const
{
    static const struct {
        const char *type;
        Qt::GlobalColor color;
    } types[] = {
        { "warp", Qt::cyan },
        { "npc", Qt::yellow },
        { "spawn", Qt::magenta },
        { "particle_effect", Qt::green },
        { 0, Qt::black }
    };

    Qt::GlobalColor color = Qt::transparent;
    const QString &type = mType;

    for (int i = 0; types[i].type; ++i) {
        if (!type.compare(QLatin1String(types[i].type), Qt::CaseInsensitive)) {
            color = types[i].color;
            break;
        }
    }

    return color;
}
