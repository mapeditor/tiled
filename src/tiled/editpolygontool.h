/*
 * editpolygontool.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QHash>
#include <QSet>

#include <memory>

class QGraphicsItem;

namespace Tiled {

class PointHandle;
class SelectionRectangle;

/**
 * A tool that allows dragging around the points of a polygon.
 */
class EditPolygonTool : public AbstractObjectTool
{
    Q_OBJECT

public:
    explicit EditPolygonTool(QObject *parent = nullptr);
    ~EditPolygonTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *event) override;

    void mouseEntered() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClicked(QGraphicsSceneMouseEvent *event) override;
    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

    bool hasSelectedHandles() const { return !mSelectedHandles.isEmpty(); }

public slots:
    void deleteNodes();

protected:
    void changeEvent(const ChangeEvent &event) override;

private:
    void updateHandles();
    void objectsAboutToBeRemoved(const QList<MapObject *> &objects);

    void joinNodes();
    void splitSegments();
    void deleteSegment();
    void extendPolyline();

    void updateHover(const QPointF &scenePos, QGraphicsSceneMouseEvent *event = nullptr);

    void setSelectedHandles(const QSet<PointHandle*> &handles);
    void setSelectedHandle(PointHandle *handle)
    { setSelectedHandles(QSet<PointHandle*>() << handle); }

    void setHighlightedHandles(const QSet<PointHandle*> &handles);

    void updateSelection(QGraphicsSceneMouseEvent *event);

    void startSelecting();

    void startMoving(const QPointF &pos);
    void updateMovingItems(const QPointF &pos,
                           Qt::KeyboardModifiers modifiers);
    void finishMoving();

    enum AbortReason {
        UserInteraction,
        ObjectsRemoved,
        Deactivated
    };

    void abortCurrentAction(AbortReason reason);

    void showHandleContextMenu(QPoint screenPos);

    QSet<PointHandle*> clickedHandles() const;

    enum Action {
        NoAction,
        Selecting,
        Moving
    };

    struct InteractedSegment {
        MapObject *object = nullptr;
        int index = 0;
        QPointF nearestPointOnLine;

        explicit operator bool() const { return object != nullptr; }
        void clear() { object = nullptr; }
    };

    std::unique_ptr<SelectionRectangle> mSelectionRectangle;
    bool mMousePressed;
    PointHandle *mHoveredHandle;
    InteractedSegment mHoveredSegment;
    PointHandle *mClickedHandle;
    InteractedSegment mClickedSegment;
    MapObject *mClickedObject;
    QVector<QPointF> mOldHandlePositions;
    QHash<MapObject*, QPolygonF> mOldPolygons;
    QPointF mAlignPosition;
    Action mAction;
    QPointF mStart;
    QPointF mLastMousePos;
    QPoint mScreenStart;
    Qt::KeyboardModifiers mModifiers;

    /// The list of handles associated with each selected map object
    QHash<MapObject*, QList<PointHandle*> > mHandles;
    QSet<PointHandle*> mSelectedHandles;
    QSet<PointHandle*> mHighlightedHandles;
};

} // namespace Tiled
