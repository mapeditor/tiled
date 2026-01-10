/*
 * tilemovetool.h
 * Copyright 2025, Tiled contributors
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
#include "tilelayer.h"

#include <QRegion>

#include <memory>

namespace Tiled {

class FloatingTileSelectionItem;

class TileMoveTool : public AbstractTileTool
{
    Q_OBJECT

public:
    explicit TileMoveTool(QObject *parent = nullptr);
    ~TileMoveTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers) override;

    void keyPressed(QKeyEvent *event) override;
    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

protected:
    void tilePositionChanged(QPoint tilePos) override;
    void updateStatusInfo() override;
    void updateEnabledState() override;
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

private:
    enum State {
        Idle,
        PickingUp,
        Moving,
    };

    void setState(State state);
    void pickUpSelection();
    void updateFloatingPosition();
    void commitMove();
    void cancelMove();
    bool hasActiveSelection() const;

    State mState = Idle;
    bool mDuplicateMode = false;

    std::unique_ptr<FloatingTileSelectionItem> mFloatingItem;
    std::unique_ptr<TileLayer> mFloatingTiles;
    QRegion mOriginalSelection;
    QPoint mPickupTilePos;
    QPoint mCurrentTilePos;

    QPointF mMouseScreenStart;
    static constexpr int DragThreshold = 3;

    int mUndoIndexBeforeMove = -1;

private slots:
    void onUndoIndexChanged();
    void onSelectionChanged();
};

} // namespace Tiled
