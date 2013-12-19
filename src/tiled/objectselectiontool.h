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
        Rotating
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

    SelectionRectangle *mSelectionRectangle;
    QGraphicsItem *mRotationOriginIndicator;
    CornerHandle *mCornerHandles[4];
    bool mMousePressed;
    MapObjectItem *mClickedObjectItem;
    CornerHandle *mClickedCornerHandle;
    QSet<MapObjectItem*> mMovingItems;
    QVector<QPointF> mOldObjectItemPositions;
    QVector<QPointF> mOldObjectPositions;
    QVector<qreal> mOldObjectRotations;
    QPointF mAlignPosition;
    QRectF mSelectionBoundingRect;
    QPointF mRotationOrigin;
    Mode mMode;
    QPointF mStart;
    QPoint mScreenStart;
    Qt::KeyboardModifiers mModifiers;
};

} // namespace Internal
} // namespace Tiled

#endif // OBJECTSELECTIONTOOL_H
