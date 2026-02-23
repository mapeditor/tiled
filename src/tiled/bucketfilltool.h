/*
 * bucketfilltool.h
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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
#include "tilelayer.h"

class QToolBar;

namespace Tiled {

class WangSet;

class MapDocument;

/**
 * Implements a tool that bucket fills (flood fills) a region with a repeatable
 * stamp.
 */
class BucketFillTool : public AbstractTileFillTool
{
    Q_OBJECT

public:
    BucketFillTool(QObject *parent = nullptr);
    ~BucketFillTool() override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void keyPressed(QKeyEvent *event) override;

    void modifiersChanged(Qt::KeyboardModifiers) override;

    void languageChanged() override;
    void populateToolBar(QToolBar *toolBar) override;

protected:
    void tilePositionChanged(QPoint tilePos) override;
    void clearConnections(MapDocument *mapDocument) override;

private:
    void setContiguous(bool contiguous);
    void clearOverlay();

    Qt::KeyboardModifiers mModifiers;
    bool mLastShiftStatus;
    bool mLastContiguousStatus = true;

    /**
     * The active fill method during the last call of tilePositionChanged().
     *
     * This variable is needed to detect if the fill method was changed during
     * mFillOverlay being brushed at an area.
     */
    FillMethod mLastFillMethod;

    bool mMouseDown = false;
    QRegion mFillRegion;
    QVector<Cell> mMatchCells;
    bool mContiguous = true;

    void makeConnections();
};

} // namespace Tiled
