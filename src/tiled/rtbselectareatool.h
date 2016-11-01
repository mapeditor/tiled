/*
 * rtbselectareatool.h
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

#ifndef RTBSELECTAREATOOL_H
#define RTBSELECTAREATOOL_H

#include "tilelayer.h"
#include "tileselectiontool.h"
#include "tilestamp.h"

class SelectedAreaItem;

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;
class MapObjectItem;

class RTBSelectAreaTool: public TileSelectionTool
{
    Q_OBJECT

public:
    RTBSelectAreaTool(QObject *parent);
    ~RTBSelectAreaTool();

    void languageChanged();

    void activate(MapScene *scene);
    void deactivate(MapScene *scene);

    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);

    void deleteArea();

    bool isActive() { return mIsActive; }

protected:
    void tilePositionChanged(const QPoint &tilePos);
    void updateEnabledState();
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument);

    /**
     * Returns the last recorded tile position of the mouse.
     */
    QPoint tilePosition() const { return QPoint(mTileX, mTileY); }

    /**
     * Updates the status info with the current tile position. When the mouse
     * is not in the view, the status info is set to an empty string.
     *
     * This behaviour can be overridden in a subclass. This method is
     * automatically called after each call to tilePositionChanged() and when
     * the brush visibility changes.
     */
    virtual void updateStatusInfo();

private:
    enum Action {
        NoAction,
        Selecting,
        Moving
    };

    enum PaintFlags {
        Mergeable               = 0x1,
        SuppressRegionEdited    = 0x2
    };

    enum BrushBehavior {
        Free,           // nothing special: you can move the mouse,
                        // preview of the selection
        Paint          // left mouse pressed: free painting
    };

    struct MovingObject
    {
        MapObjectItem *item;
        QPointF oldItemPosition;

        QPointF oldPosition;
        QSizeF oldSize;
        QPolygonF oldPolygon;
        qreal oldRotation;
    };

    void startMoving(Qt::KeyboardModifiers modifiers);
    void updateMovingItems(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void finishMoving(const QPointF &pos);

    const QPointF snapToGrid(const QPointF &diff, Qt::KeyboardModifiers modifiers);
    void saveSelectionState();
    void updateSelection(Qt::KeyboardModifiers modifiers);
    QList<MapObject *> changingObjects() const;
    void updateMousePosition(const QPointF &pos);
    void removeSelectedAreaItems();

    void setStamp(const TileStamp &stamp);
    void updatePreview();
    void updatePreview(QPoint tilePos);
    void beginPaint();
    QRegion doPaint(int flags = 0);
    void drawPreviewLayer(const QVector<QPoint> &list);


    int mTileX, mTileY;
    bool mIsActive;
    bool mMousePressed;
    Action mAction;
    MapScene *mMapScene;
    QVector<MovingObject> mMovingObjects;
    QPointF mAlignPosition;
    QPointF mStart;
    QPointF mDragDelta;

    BrushBehavior mBrushBehavior;
    TileStamp mStamp;
    SharedTileLayer mPreviewLayer;
    QVector<SharedTileset> mMissingTilesets;
    QPoint mPrevTilePosition;

    QList<SelectedAreaItem*> mSelectedAreaItems;
    TileLayer *mEditedRegion;

};

} // namespace Internal
} // namespace Tiled

#endif // RTBSELECTAREATOOL_H
