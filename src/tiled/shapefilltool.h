/*
 * shapefilltool.h
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

#include "abstracttilefilltool.h"

class QAction;

namespace Tiled {

class ShapeFillTool : public AbstractTileFillTool
{
    Q_OBJECT

public:
    ShapeFillTool(QObject *parent = nullptr);

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void modifiersChanged(Qt::KeyboardModifiers) override;

    void languageChanged() override;

    void populateToolBar(QToolBar *toolBar) override;

protected:
    void tilePositionChanged(QPoint) override;
    void clearConnections(MapDocument *) override {}

private:
    enum ToolBehavior {
        Free,   // nothing has been started
        MakingShape
    };

    enum Shape {
        Rect,   // making a rectangle
        Circle  // making a circle
    };

    ToolBehavior mToolBehavior;
    Shape mCurrentShape;
    QPoint mStartCorner;

    QAction *mRectFill;
    QAction *mCircleFill;

    void setCurrentShape(Shape shape);
    void updateFillOverlay();
};

} // namespace Tiled
