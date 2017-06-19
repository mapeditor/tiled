/*
 * moveselectiontool.cpp
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

#include "moveselectiontool.h"

#include "brushitem.h"
#include "changeselectedarea.h"
#include "erasetiles.h"
#include "mapdocument.h"
#include "painttilelayer.h"
#include "tilesetmanager.h"

#include <QApplication>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

MoveSelectionTool::MoveSelectionTool(QObject *parent)
    : AbstractTileTool(tr("Move Selection"),
                       QIcon(QLatin1String(
                               ":images/22x22/move-selection.png")),
                       QKeySequence(tr("G")),
                       parent)
    , mDragging(false)
    , mMouseDown(false)
    , mDuplicate(false)
{
}

MoveSelectionTool::~MoveSelectionTool()
{
}

void MoveSelectionTool::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);

    brushItem()->setTileRegion(mapDocument()->selectedArea());
    brushItem()->setVisible(true);
    mTargetLayer = mapDocument()->currentLayer()->asTileLayer();

    makeConnections();
}

void MoveSelectionTool::deactivate(MapScene *scene)
{
    AbstractTileTool::deactivate(scene);
    paste();
}

void MoveSelectionTool::mouseEntered()
{
}

void MoveSelectionTool::mouseLeft()
{
}

void MoveSelectionTool::tilePositionChanged(const QPoint &pos)
{
    if (mDragging) {
        QPoint offset = pos - mLastUpdate;

        if (mPreviewLayer) {
            mPreviewLayer->setX(mPreviewLayer->x() + offset.x());
            mPreviewLayer->setY(mPreviewLayer->y() + offset.y());
            brushItem()->setTileLayer(mPreviewLayer,
                                      brushItem()->tileRegion().translated(offset));
        }

        mLastUpdate = pos;
    }
}

void MoveSelectionTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (mMouseDown && !mDragging) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mMouseScreenStart - screenPos).manhattanLength();

        if (dragDistance >= QApplication::startDragDistance() / 2) {
            if (!mPreviewLayer) {
                cut();
            }
            mDragging = true;
            tilePositionChanged(tilePosition());
        }
    }

    AbstractTileTool::mouseMoved(pos, modifiers);

    refreshCursor();
}

void MoveSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (brushItem()->tileRegion().contains(tilePosition())) {
            mMouseScreenStart = event->screenPos();
            mDragStart = tilePosition();
            mMouseDown = true;
            mLastUpdate = tilePosition();
        } else {
            paste();
        }
    }
}

void MoveSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    mDragging = false;
    mMouseDown = false;
    refreshCursor();
}

void MoveSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mDuplicate = modifiers & Qt::ControlModifier;
}

void MoveSelectionTool::languageChanged()
{
    setName(tr("Move Selection"));
    setShortcut(QKeySequence(tr("G")));
}

void MoveSelectionTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    if (brushItem()->tileRegion().contains(tilePosition()) || mDragging)
        cursorShape = Qt::SizeAllCursor;

    if (cursor().shape() != cursorShape)
        setCursor(cursorShape);
}

void MoveSelectionTool::makeConnections()
{
    if (!mapDocument())
        return;

    connect(mapDocument(), &MapDocument::layerRemoved, this, &MoveSelectionTool::layerRemoved);
}

void MoveSelectionTool::layerRemoved(Layer *layer)
{
    if (layer == mTargetLayer)
        mTargetLayer = nullptr;
}

void MoveSelectionTool::cut()
{
    const QRegion &selectedArea = mapDocument()->selectedArea();

    mPreviewLayer = SharedTileLayer(mTargetLayer->copy(selectedArea.translated(-mTargetLayer->position())));
    mPreviewLayer->setPosition(selectedArea.boundingRect().topLeft());

    brushItem()->setTileLayer(mPreviewLayer, selectedArea);

    QUndoStack *stack = mapDocument()->undoStack();

    stack->beginMacro(tr("Move Selection"));

    if (!mDuplicate)
        stack->push(new EraseTiles(mapDocument(), mTargetLayer, selectedArea));

    stack->push(new ChangeSelectedArea(mapDocument(), QRegion()));

    stack->endMacro();
}

void MoveSelectionTool::paste()
{
    const TileLayer *preview = mPreviewLayer.data();
    if (!preview || !mTargetLayer)
        return;

    auto undoStack = mapDocument()->undoStack();

    undoStack->push(new PaintTileLayer(mapDocument(),
                                       mTargetLayer,
                                       preview->x(),
                                       preview->y(),
                                       preview,
                                       brushItem()->tileRegion()));

    brushItem()->clear();
    mPreviewLayer.clear();
}
