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

#pragma once

#include "abstractobjecttool.h"

#include <QList>
#include <QSet>
#include <QVector>

class QGraphicsItem;

namespace Tiled {
namespace Internal {

class Handle;
class MapObjectItem;
class OriginIndicator;
class ResizeHandle;
class RotateHandle;
class SelectionRectangle;

class ObjectSelectionTool : public AbstractObjectTool
{
    Q_OBJECT

public:
    explicit ObjectSelectionTool(QObject *parent = nullptr);
    ~ObjectSelectionTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *) override;
    void mouseEntered() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

private slots:
    void updateHandles(bool resetOriginIndicator = true);
    void updateHandleVisibility();

    void objectsRemoved(const QList<MapObject *> &);

private:
    enum Action {
        NoAction,
        Selecting,
        Moving,
        MovingOrigin,
        Rotating,
        Resizing
    };

    enum Mode {
        Resize,
        Rotate,
    };

    void updateHover(const QPointF &pos);
    void updateSelection(const QPointF &pos,
                         Qt::KeyboardModifiers modifiers);

    void startSelecting();

    void startMoving(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void updateMovingItems(const QPointF &pos,
                           Qt::KeyboardModifiers modifiers);
    void finishMoving(const QPointF &pos);

    void startMovingOrigin(const QPointF &pos);
    void updateMovingOrigin(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void finishMovingOrigin();

    void startRotating(const QPointF &pos);
    void updateRotatingItems(const QPointF &pos,
                             Qt::KeyboardModifiers modifiers);
    void finishRotating(const QPointF &pos);

    void startResizing();
    void updateResizingItems(const QPointF &pos,
                             Qt::KeyboardModifiers modifiers);
    void updateResizingSingleItem(const QPointF &resizingOrigin,
                                  const QPointF &screenPos,
                                  Qt::KeyboardModifiers modifiers);
    void finishResizing(const QPointF &pos);

    void setMode(Mode mode);
    void saveSelectionState();

    void refreshCursor();

    QPointF snapToGrid(const QPointF &pos,
                       Qt::KeyboardModifiers modifiers);

    QList<MapObject*> changingObjects() const;

    struct MovingObject
    {
        MapObject *mapObject;
        QPointF oldScreenPosition;

        QPointF oldPosition;
        QSizeF oldSize;
        QPolygonF oldPolygon;
        qreal oldRotation;
    };

    SelectionRectangle *mSelectionRectangle;
    QGraphicsItem *mOriginIndicator;
    RotateHandle *mRotateHandles[4];
    ResizeHandle *mResizeHandles[8];
    bool mMousePressed;

    MapObjectItem *mHoveredObjectItem;
    Handle *mHoveredHandle;

    MapObjectItem *mClickedObjectItem;
    OriginIndicator *mClickedOriginIndicator;
    RotateHandle *mClickedRotateHandle;
    ResizeHandle *mClickedResizeHandle;

    QVector<MovingObject> mMovingObjects;

    QPointF mOldOriginPosition;

    QPointF mAlignPosition;
    QPointF mOrigin;
    bool mResizingLimitHorizontal;
    bool mResizingLimitVertical;
    Mode mMode;
    Action mAction;
    QPointF mStart;
    QPointF mStartOffset;
    QPoint mScreenStart;
    Qt::KeyboardModifiers mModifiers;
};

} // namespace Internal
} // namespace Tiled
