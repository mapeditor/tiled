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

#include <QMap>
#include <QSet>

class QGraphicsItem;

namespace Tiled {
namespace Internal {

class MapObjectItem;
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

    void mouseEntered() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

    bool hasSelectedHandles() const { return !mSelectedHandles.isEmpty(); }

public slots:
    void deleteNodes();

private slots:
    void updateHandles();
    void objectsRemoved(const QList<MapObject *> &objects);

    void joinNodes();
    void splitSegments();
    void deleteSegment();

private:
    enum Mode {
        NoMode,
        Selecting,
        Moving
    };

    void setSelectedHandles(const QSet<PointHandle*> &handles);
    void setSelectedHandle(PointHandle *handle)
    { setSelectedHandles(QSet<PointHandle*>() << handle); }

    void updateSelection(QGraphicsSceneMouseEvent *event);

    void startSelecting();

    void startMoving();
    void updateMovingItems(const QPointF &pos,
                           Qt::KeyboardModifiers modifiers);
    void finishMoving(const QPointF &pos);

    void showHandleContextMenu(PointHandle *clickedHandle, QPoint screenPos);

    SelectionRectangle *mSelectionRectangle;
    bool mMousePressed;
    PointHandle *mClickedHandle;
    MapObjectItem *mClickedObjectItem;
    QVector<QPointF> mOldHandlePositions;
    QMap<MapObject*, QPolygonF> mOldPolygons;
    QPointF mAlignPosition;
    Mode mMode;
    QPointF mStart;
    QPoint mScreenStart;
    Qt::KeyboardModifiers mModifiers;

    /// The list of handles associated with each selected map object
    QMap<MapObject*, QList<PointHandle*> > mHandles;
    QSet<PointHandle*> mSelectedHandles;
};

} // namespace Internal
} // namespace Tiled
