/*
 * wangbrush.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 * Copyright 2020, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "abstracttiletool.h"
#include "brushitem.h"
#include "wangfiller.h"
#include "wangset.h"

namespace Tiled {

class WangBrushItem : public BrushItem
{
public:
    WangBrushItem() {}

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setInvalidTiles(const QRegion &region);
    bool isValid() const { return mInvalidTiles.isEmpty(); }

private:
    // The tiles which can't be painted.
    QRegion mInvalidTiles;
};

class WangBrush : public AbstractTileTool
{
    Q_OBJECT

public:
    enum BrushMode {
        PaintCorner,
        PaintEdge,
        PaintEdgeAndCorner,
        Idle // no valid color selected
    };

    WangBrush(QObject *parent = nullptr);
    ~WangBrush() override;

    void activate(MapScene *scene) override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers) override;

    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

    void setColor(int color);

signals:
    void colorCaptured(int color);

public slots:
    void wangSetChanged(const WangSet *wangSet);

protected:
    void tilePositionChanged(QPoint tilePos) override;
    void mapDocumentChanged(MapDocument *oldDocument, MapDocument *newDocument) override;
    void updateStatusInfo() override;

private:
    enum BrushBehavior {
        Free,               // Hovering
        Paint,              // Painting (left mouse button pressed)
        Line,               // Drawing a line (left mouse button pressed)
    };

    // sets the current wang color to the corner/edge currently hovered
    void captureHoverColor();

    // called when something has changed which requires an update.
    void stateChanged();

    void beginPaint();
    void doPaint(bool mergeable);
    void updateBrush();
    void updateBrushAt(WangFiller &filler, QPoint pos);

    // The point painting happens around
    // In tile mode, this is that tile
    // In corner mode, this means the top-left corner of that tile
    // In edge mode, this is a tile with that edge
    // With mWangIndex being the direction of the edge
    QPoint mPrevPaintPoint;
    QPoint mPaintPoint;
    QPoint mLineStartPos;
    WangId::Index mWangIndex = WangId::Top;

    const WangSet *mWangSet = nullptr;
    int mCurrentColor = 0;
    BrushMode mBrushMode = Idle;
    bool mIsTileMode = false;
    bool mRotationalSymmetry = false;
    bool mLineStartSet = false;
    BrushBehavior mBrushBehavior = Free;
};

} // namespace Tiled
