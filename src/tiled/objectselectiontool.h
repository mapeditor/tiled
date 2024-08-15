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

#include <array>
#include <memory>

class QGraphicsItem;

namespace Tiled {

class Handle;
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
    void mouseLeft() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClicked(QGraphicsSceneMouseEvent *event) override;
    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

    void populateToolBar(QToolBar*) override;

protected:
    void changeEvent(const ChangeEvent &event) override;

private:
    void languageChangedImpl();

    void updateHandles();
    void updateHandlesAndOrigin();
    void updateHandleVisibility();

    void objectsAboutToBeRemoved(const QList<MapObject *> &);

    void setSelectionMode(Qt::ItemSelectionMode selectionMode);

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

    void updateHandlesImpl(bool resetOriginIndicator);

    void updateHover(const QPointF &pos);
    QList<MapObject*> objectsAboutToBeSelected(const QPointF &pos,
                                               Qt::KeyboardModifiers modifiers) const;
    void updateSelection(const QPointF &pos, Qt::KeyboardModifiers modifiers);

    void startSelecting();

    void startMoving(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void updateMovingItems(const QPointF &pos,
                           Qt::KeyboardModifiers modifiers);
    void finishMoving();

    void startMovingOrigin(const QPointF &pos);
    void updateMovingOrigin(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void finishMovingOrigin();

    void startRotating(const QPointF &pos);
    void updateRotatingItems(const QPointF &pos,
                             Qt::KeyboardModifiers modifiers);
    void finishRotating();

    void startResizing();
    void updateResizingItems(const QPointF &pos,
                             Qt::KeyboardModifiers modifiers);
    void updateResizingSingleItem(const QPointF &resizingOrigin,
                                  const QPointF &screenPos,
                                  Qt::KeyboardModifiers modifiers);
    void finishResizing();

    void setMode(Mode mode);
    void saveSelectionState();

    enum AbortReason {
        UserInteraction,
        ObjectsRemoved,
        Deactivated
    };

    void abortCurrentAction(AbortReason reason);

    void refreshCursor();

    QPointF snapToGrid(const QPointF &pos,
                       Qt::KeyboardModifiers modifiers);

    QList<MapObject*> changingObjects() const;

    QAction *mSelectIntersected;
    QAction *mSelectContained;

    std::unique_ptr<SelectionRectangle> mSelectionRectangle;
    std::unique_ptr<QGraphicsItem> mOriginIndicator;
    std::array<RotateHandle*, 4> mRotateHandles;
    std::array<ResizeHandle*, 8> mResizeHandles;
    bool mMousePressed = false;

    MapObject *mHoveredObject = nullptr;
    Handle *mHoveredHandle = nullptr;

    MapObject *mClickedObject = nullptr;
    OriginIndicator *mClickedOriginIndicator = nullptr;
    RotateHandle *mClickedRotateHandle = nullptr;
    ResizeHandle *mClickedResizeHandle = nullptr;

    struct MovingObject
    {
        MapObject *mapObject;
        QPointF oldScreenPosition;

        QPointF oldPosition;
        QSizeF oldSize;
        QPolygonF oldPolygon;
        qreal oldRotation;
    };

    QVector<MovingObject> mMovingObjects;

    QPointF mAlignPosition;
    QPointF mOriginPos;
    bool mResizingLimitHorizontal = false;
    bool mResizingLimitVertical = false;
    Qt::ItemSelectionMode mSelectionMode;
    Mode mMode = Resize;
    Action mAction = NoAction;
    QPointF mStart;
    QPointF mStartOffset;
    QPointF mLastMousePos;
    QPoint mScreenStart;
    Qt::KeyboardModifiers mModifiers;

    static Preference<Qt::ItemSelectionMode> ourSelectionMode;
};

} // namespace Tiled
