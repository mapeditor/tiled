/*
 * wangbrush.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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
#include "wangset.h"

namespace Tiled {

class WangBrush : public AbstractTileTool
{
    Q_OBJECT

public:
    enum BrushMode {
        PaintVertex,
        PaintEdge,
        Idle //no valid color selected
    };

    WangBrush(QObject *parent = nullptr);
    ~WangBrush();

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers) override;

    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

    //Sets to edge mode, with color
    void setEdgeColor(int color);
    //Sets to color mode, with color
    void setCornerColor(int color);

protected:
    void tilePositionChanged(QPoint tilePos) override;
    void mapDocumentChanged(MapDocument *oldDocument, MapDocument *newDocument) override;
    void updateStatusInfo() override;

signals:
    void colorCaptured(int color, bool isEdge);

public slots:
    void wangColorChanged(int color, bool edge);
    void wangSetChanged(WangSet *wangSet);

private:
    enum BrushBehavior {
        Free,
        Paint
    };

    //sets the current wang color to the corner/edge currently hovered
    void captureHoverColor();

    //called when something has changed which requires an update.
    void stateChanged();

    void beginPaint();
    void doPaint(bool mergeable);
    void updateBrush();

    //The point painting happens around
    //In tile mode, this is that tile
    //In vertex mode, this is that vertex
    //In edge mode, this is a tile with that edge
    //With mEdge being the direction of the edge (0 being top 3 being left)
    QPoint mPaintPoint;
    WangId::Edge mEdgeDir;

    WangSet *mWangSet;
    int mCurrentColor;
    BrushMode mBrushMode;
    bool mIsTileMode;
    BrushBehavior mBrushBehavior;
};

} // namespace Tiled
