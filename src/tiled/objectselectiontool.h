/*
 * objectselectiontool.h
 * Copyright 2010-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef OBJECTSELECTIONTOOL_H
#define OBJECTSELECTIONTOOL_H

#include "abstractobjecttool.h"

#include <QSet>

class QGraphicsItem;

namespace Tiled {
namespace Internal {

class CornerHandle;
class ResizeHandle;
class MapObjectItem;
class SelectionRectangle;

class ObjectSelectionTool : public AbstractObjectTool
{
    Q_OBJECT

public:
    explicit ObjectSelectionTool(QObject *parent = 0);
    ~ObjectSelectionTool();

    void activate(MapScene *scene);
    void deactivate(MapScene *scene);

    void keyPressed(QKeyEvent *);
    void mouseEntered();
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);
    void modifiersChanged(Qt::KeyboardModifiers modifiers);

    void languageChanged();

private slots:
    void updateHandles();
    void setHandlesVisible(bool visible);

    void objectsRemoved(const QList<MapObject *> &);

private:
    enum Mode {
        NoMode,
        Selecting,
        Moving,
        Rotating,
        Resizing
    };

    void updateSelection(const QPointF &pos,
                         Qt::KeyboardModifiers modifiers);

    void startSelecting();

    void startMoving();
    void updateMovingItems(const QPointF &pos,
                           Qt::KeyboardModifiers modifiers);
    void finishMoving(const QPointF &pos);

    void startRotating();
    void updateRotatingItems(const QPointF &pos,
                             Qt::KeyboardModifiers modifiers);
    void finishRotating(const QPointF &pos);
    
    void startResizing();
    void updateResizingItems(const QPointF &pos,
                             Qt::KeyboardModifiers modifiers);
    void updateResizingSingleItem(const QPointF &pos,
                             Qt::KeyboardModifiers modifiers);
    void finishResizing(const QPointF &pos);
    
    const QPointF snapToGrid(const QPointF &pos,
                             Qt::KeyboardModifiers modifiers);

    SelectionRectangle *mSelectionRectangle;
    QGraphicsItem *mRotationOriginIndicator;
    CornerHandle *mCornerHandles[4];
    ResizeHandle *mResizeHandles[8];
    bool mMousePressed;
    MapObjectItem *mClickedObjectItem;
    CornerHandle *mClickedCornerHandle;
    ResizeHandle *mClickedResizeHandle;
    QSet<MapObjectItem*> mMovingItems;
    QVector<QPointF> mOldObjectItemPositions;
    QVector<QPointF> mOldObjectPositions;
    QVector<QSizeF> mOldObjectSizes;
    QVector<qreal> mOldObjectRotations;
    QPointF mAlignPosition;
    QRectF mSelectionBoundingRect;
    QPointF mRotationOrigin;
    QPointF mResizingOrigin;
    bool mResizingLimitHorizontal;
    bool mResizingLimitVertical;
    Mode mMode;
    QPointF mStart;
    Qt::KeyboardModifiers mModifiers;
};

} // namespace Internal
} // namespace Tiled

#endif // OBJECTSELECTIONTOOL_H
