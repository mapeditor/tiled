/*
 * tilemovetool.cpp
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

#include "tilemovetool.h"

#include "brushitem.h"
#include "changeselectedarea.h"
#include "floatingtileselectionitem.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "movetiles.h"
#include "tilelayer.h"

#include <QApplication>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QUndoStack>

using namespace Tiled;

TileMoveTool::TileMoveTool(QObject *parent)
    : AbstractTileTool("TileMoveTool",
                       tr("Move Tiles"),
                       QIcon(QLatin1String(":images/22/stock-tool-move-22.png")),
                       QKeySequence(Qt::Key_V),
                       nullptr,
                       parent)
{
    setUsesSelectedTiles(true);
}

TileMoveTool::~TileMoveTool()
{
}

void TileMoveTool::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);

    mFloatingItem = std::make_unique<FloatingTileSelectionItem>(mapDocument());
    mFloatingItem->setZValue(10000);
    scene->addItem(mFloatingItem.get());
}

void TileMoveTool::deactivate(MapScene *scene)
{
    if (mState == Moving)
        cancelMove();

    if (mFloatingItem) {
        scene->removeItem(mFloatingItem.get());
        mFloatingItem.reset();
    }

    AbstractTileTool::deactivate(scene);
}

void TileMoveTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (mState == Idle && hasActiveSelection()) {
            mMouseScreenStart = event->screenPos();
            mPickupTilePos = tilePosition();
            mDuplicateMode = event->modifiers() & Qt::AltModifier;
            setState(PickingUp);
            return;
        }
    }

    if (event->button() == Qt::RightButton && mState == Moving) {
        cancelMove();
        return;
    }

    AbstractTileTool::mousePressed(event);
}

void TileMoveTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        switch (mState) {
        case Idle:
            break;
        case PickingUp:
            setState(Idle);
            break;
        case Moving:
            commitMove();
            break;
        }
    }
}

void TileMoveTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    AbstractTileTool::mouseMoved(pos, modifiers);

    if (mState == PickingUp) {
        QPointF screenPos = mapScene()->views().first()->mapFromScene(pos);
        QPointF delta = screenPos - mMouseScreenStart;
        if (delta.manhattanLength() >= DragThreshold) {
            pickUpSelection();
        }
    } else if (mState == Moving) {
        updateFloatingPosition();
    }
}

void TileMoveTool::keyPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && mState == Moving) {
        cancelMove();
        return;
    }

    AbstractTileTool::keyPressed(event);
}

void TileMoveTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    AbstractTileTool::modifiersChanged(modifiers);

    bool wasDuplicate = mDuplicateMode;
    mDuplicateMode = modifiers & Qt::AltModifier;

    if (mState == Moving && wasDuplicate != mDuplicateMode) {
        updateStatusInfo();
    }

    if (mState == Moving) {
        setCursor(mDuplicateMode ? Qt::DragCopyCursor : Qt::SizeAllCursor);
    } else if (hasActiveSelection()) {
        setCursor(Qt::SizeAllCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void TileMoveTool::languageChanged()
{
    setName(tr("Move Tiles"));
}

void TileMoveTool::tilePositionChanged(QPoint)
{
    if (mState == Moving) {
        updateFloatingPosition();
    }

    updateStatusInfo();
}

void TileMoveTool::updateStatusInfo()
{
    if (!isBrushVisible()) {
        AbstractTileTool::updateStatusInfo();
        return;
    }

    const QPoint pos = tilePosition();

    if (mState == Moving) {
        QPoint offset = pos - mPickupTilePos;
        QRect bounds = mOriginalSelection.boundingRect();
        QString action = mDuplicateMode ? tr("Duplicating") : tr("Moving");
        setStatusInfo(tr("%1, %2 - %3 (%4 x %5) by (%6, %7)")
                          .arg(pos.x())
                          .arg(pos.y())
                          .arg(action)
                          .arg(bounds.width())
                          .arg(bounds.height())
                          .arg(offset.x())
                          .arg(offset.y()));
    } else if (hasActiveSelection()) {
        setStatusInfo(tr("%1, %2 (Drag to move, Alt+drag to duplicate)")
                          .arg(pos.x())
                          .arg(pos.y()));
    } else {
        setStatusInfo(tr("%1, %2 (Select tiles first)")
                          .arg(pos.x())
                          .arg(pos.y()));
    }
}

void TileMoveTool::updateEnabledState()
{
    AbstractTileTool::updateEnabledState();
}

void TileMoveTool::mapDocumentChanged(MapDocument *oldDocument,
                                      MapDocument *newDocument)
{
    if (mState == Moving) {
        cancelMove();
    }

    if (oldDocument) {
        disconnect(oldDocument, &MapDocument::selectedAreaChanged,
                   this, &TileMoveTool::onSelectionChanged);
    }

    if (newDocument) {
        connect(newDocument, &MapDocument::selectedAreaChanged,
                this, &TileMoveTool::onSelectionChanged);
    }

    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);
}

void TileMoveTool::setState(State state)
{
    if (mState == state)
        return;

    mState = state;

    switch (state) {
    case Idle:
        setCursor(hasActiveSelection() ? Qt::SizeAllCursor : Qt::ArrowCursor);
        break;
    case PickingUp:
        setCursor(Qt::ClosedHandCursor);
        break;
    case Moving:
        setCursor(mDuplicateMode ? Qt::DragCopyCursor : Qt::SizeAllCursor);
        break;
    }

    updateStatusInfo();
}

void TileMoveTool::pickUpSelection()
{
    if (!mapDocument())
        return;

    TileLayer *layer = currentTileLayer();
    if (!layer)
        return;

    mOriginalSelection = mapDocument()->selectedArea();
    if (mOriginalSelection.isEmpty()) {
        setState(Idle);
        return;
    }

    mFloatingTiles = std::make_unique<TileLayer>();
    mFloatingTiles->setCells(0, 0, layer, mOriginalSelection);

    mFloatingItem->setTiles(mFloatingTiles.get(), mOriginalSelection);
    mFloatingItem->setTileOffset(QPoint(0, 0));

    if (!mDuplicateMode) {
        brushItem()->setTileLayer(nullptr);
        brushItem()->setTileRegion(mOriginalSelection);
    }

    mUndoIndexBeforeMove = mapDocument()->undoStack()->index();
    connect(mapDocument()->undoStack(), &QUndoStack::indexChanged,
            this, &TileMoveTool::onUndoIndexChanged);

    mCurrentTilePos = mPickupTilePos;
    setState(Moving);
}

void TileMoveTool::updateFloatingPosition()
{
    if (!mFloatingItem)
        return;

    QPoint offset = tilePosition() - mPickupTilePos;
    mFloatingItem->setTileOffset(offset);
    mCurrentTilePos = tilePosition();
}

void TileMoveTool::commitMove()
{
    if (!mapDocument() || mOriginalSelection.isEmpty()) {
        cancelMove();
        return;
    }

    TileLayer *layer = currentTileLayer();
    if (!layer) {
        cancelMove();
        return;
    }

    disconnect(mapDocument()->undoStack(), &QUndoStack::indexChanged,
               this, &TileMoveTool::onUndoIndexChanged);

    QPoint offset = mCurrentTilePos - mPickupTilePos;

    if (offset != QPoint(0, 0) || mDuplicateMode) {
        QUndoStack *undoStack = mapDocument()->undoStack();

        undoStack->beginMacro(mDuplicateMode ? tr("Duplicate Tiles") : tr("Move Tiles"));

        undoStack->push(new MoveTiles(
            mapDocument(),
            layer,
            mOriginalSelection,
            offset,
            mDuplicateMode
        ));

        QRegion newSelection = mOriginalSelection.translated(offset);
        undoStack->push(new ChangeSelectedArea(mapDocument(), newSelection));

        undoStack->endMacro();
    }

    mFloatingItem->setTiles(nullptr, QRegion());
    mFloatingTiles.reset();
    mOriginalSelection = QRegion();
    brushItem()->setTileRegion(QRegion());

    setState(Idle);
}

void TileMoveTool::cancelMove()
{
    if (mapDocument()) {
        disconnect(mapDocument()->undoStack(), &QUndoStack::indexChanged,
                   this, &TileMoveTool::onUndoIndexChanged);
    }

    if (mFloatingItem)
        mFloatingItem->setTiles(nullptr, QRegion());
    mFloatingTiles.reset();
    mOriginalSelection = QRegion();
    brushItem()->setTileRegion(QRegion());

    setState(Idle);
}

bool TileMoveTool::hasActiveSelection() const
{
    return mapDocument() && !mapDocument()->selectedArea().isEmpty();
}

void TileMoveTool::onUndoIndexChanged()
{
    if (mState == Moving && mapDocument()) {
        int currentIndex = mapDocument()->undoStack()->index();
        if (currentIndex < mUndoIndexBeforeMove) {
            cancelMove();
        }
    }
}

void TileMoveTool::onSelectionChanged()
{
    if (mState == Moving) {
        cancelMove();
    }
}

#include "moc_tilemovetool.cpp"
