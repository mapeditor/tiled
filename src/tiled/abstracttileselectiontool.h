/*
 * abstracttileselectiontool.h
 * Copyright 2017, Ketan Gupta <ketan19972010@gmail.com>
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

class QAction;
class QActionGroup;

namespace Tiled {

class MapDocument;

class AbstractTileSelectionTool : public AbstractTileTool
{
    Q_OBJECT

public:
    AbstractTileSelectionTool(Id id,
                              const QString &name,
                              const QIcon &icon,
                              const QKeySequence &shortcut,
                              QObject *parent = nullptr);
    ~AbstractTileSelectionTool() override;

    void deactivate(MapScene *scene) override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers) override;

    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void keyPressed(QKeyEvent *event) override;

    void languageChanged() override;

    void populateToolBar(QToolBar *toolBar) override;

protected:
    enum SelectionMode {
        Replace,
        Add,
        Subtract,
        Intersect
    };

    enum MoveState {
        NoMove,
        PickingUp,
        MovingTiles,
    };

    SelectionMode selectionMode() const { return mSelectionMode; }

    bool isMovingTiles() const { return mMoveState != NoMove; }
    bool tryStartMove(QGraphicsSceneMouseEvent *event);

    const QRegion &selectionPreviewRegion() const;
    void setSelectionPreview(const QRegion &region);
    void applySelectionPreview();

    void changeSelectedArea(const QRegion &region);

    void updateBrushVisibility() override;

    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

    bool mMouseDown = false;

private:
    void pickUpSelection();
    void updateFloatingPosition();
    void commitMove();
    void cancelMove();
    bool hasActiveSelection() const;
    void updateMoveCursor();

    SelectionMode mSelectionMode;
    SelectionMode mDefaultMode;

    QAction *mReplace;
    QAction *mAdd;
    QAction *mSubtract;
    QAction *mIntersect;
    QActionGroup *mActionGroup;

    MoveState mMoveState = NoMove;
    bool mDuplicateMode = false;

    SharedTileLayer mFloatingTiles;
    QRegion mOriginalSelection;
    QPoint mPickupTilePos;
    QPoint mCurrentTilePos;
    QPointF mMoveScreenStart;
    int mUndoIndexBeforeMove = -1;

    static constexpr int MoveDragThreshold = 3;

private slots:
    void onUndoIndexChanged();
    void onMoveSelectionChanged();
};

} // namespace Tiled
