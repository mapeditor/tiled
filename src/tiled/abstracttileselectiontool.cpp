/*
 * abstracttileselectiontool.cpp
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

#include "abstracttileselectiontool.h"

#include "brushitem.h"
#include "changeselectedarea.h"
#include "erasetiles.h"
#include "mapdocument.h"
#include "painttilelayer.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QKeyEvent>
#include <QToolBar>

using namespace Tiled;
using namespace Tiled::Internal;

AbstractTileSelectionTool::AbstractTileSelectionTool(const QString &name,
                                                     const QIcon &icon,
                                                     const QKeySequence &shortcut,
                                                     QObject *parent)
    : AbstractTileTool(name, icon, shortcut, parent)
    , mDragModifier(false)
    , mDragging(false)
    , mMousePressed(false)
    , mSelectionMode(Replace)
    , mDefaultMode(Replace)
{
    QIcon replaceIcon(QLatin1String(":images/16x16/selection-replace.png"));
    QIcon addIcon(QLatin1String(":images/16x16/selection-add.png"));
    QIcon subtractIcon(QLatin1String(":images/16x16/selection-subtract.png"));
    QIcon intersectIcon(QLatin1String(":images/16x16/selection-intersect.png"));

    mReplace = new QAction(this);
    mReplace->setIcon(replaceIcon);
    mReplace->setCheckable(true);
    mReplace->setChecked(true);

    mAdd = new QAction(this);
    mAdd->setIcon(addIcon);
    mAdd->setCheckable(true);

    mSubtract = new QAction(this);
    mSubtract->setIcon(subtractIcon);
    mSubtract->setCheckable(true);

    mIntersect = new QAction(this);
    mIntersect->setIcon(intersectIcon);
    mIntersect->setCheckable(true);

    mActionGroup = new QActionGroup(this);
    mActionGroup->addAction(mReplace);
    mActionGroup->addAction(mAdd);
    mActionGroup->addAction(mSubtract);
    mActionGroup->addAction(mIntersect);

    connect(mReplace, &QAction::triggered,
            [this]() { mSelectionMode = mDefaultMode = Replace; });
    connect(mAdd, &QAction::triggered,
            [this]() { mSelectionMode = mDefaultMode = Add; });
    connect(mSubtract, &QAction::triggered,
            [this]() { mSelectionMode = mDefaultMode = Subtract; });
    connect(mIntersect, &QAction::triggered,
            [this]() { mSelectionMode = mDefaultMode = Intersect; });

    languageChanged();
}

void AbstractTileSelectionTool::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);
    setBrushVisible(true);
}

void AbstractTileSelectionTool::deactivate(MapScene *scene)
{
    if (mPreviewLayer)
        anchorSelection();

    AbstractTileTool::deactivate(scene);
}

void AbstractTileSelectionTool::keyPressed(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (mPreviewLayer) {
            anchorSelection();
            return;
        }
        break;
    }

    AbstractTileTool::keyPressed(event);
}

void AbstractTileSelectionTool::tilePositionChanged(const QPoint &pos)
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

void AbstractTileSelectionTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (mMousePressed && !mDragging) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mMouseStart - screenPos).manhattanLength();

        if (dragDistance >= QApplication::startDragDistance() / 2) {
            if (!mPreviewLayer)
                floatSelection(modifiers & Qt::ControlModifier);

            mDragging = true;
            tilePositionChanged(tilePosition());
        }
    }

    AbstractTileTool::mouseMoved(pos, modifiers);

    refreshCursor();
}

void AbstractTileSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    const Qt::MouseButton button = event->button();

    if (button != Qt::LeftButton && button != Qt::RightButton)
        return;

    MapDocument *document = mapDocument();

    if (mDragModifier || mPreviewLayer) {
        if (button == Qt::LeftButton) {
            bool startDrag = event->modifiers() & Qt::AltModifier;

            if (!startDrag && mPreviewLayer) {
                if (brushItem()->tileRegion().contains(tilePosition()))
                    startDrag = true;
                else
                    anchorSelection();
            }

            if (startDrag) {
                mMouseStart = event->screenPos();
                mDragStart = tilePosition();
                mMousePressed = true;
                mLastUpdate = tilePosition();
            }
        }

        if (button == Qt::RightButton)
            anchorSelection();

        return;
    }

    QRegion selection;

    // Left button modifies selection, right button clears selection
    if (button == Qt::LeftButton) {
        selection = document->selectedArea();

        switch (mSelectionMode) {
        case Replace:   selection = mSelectedRegion; break;
        case Add:       selection += mSelectedRegion; break;
        case Subtract:  selection -= mSelectedRegion; break;
        case Intersect: selection &= mSelectedRegion; break;
        }
    }

    if (selection != document->selectedArea()) {
        QUndoCommand *cmd = new ChangeSelectedArea(document, selection);
        document->undoStack()->push(cmd);
    }
}

void AbstractTileSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    mDragging = false;
    mMousePressed = false;
    refreshCursor();
}

void AbstractTileSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if (modifiers == Qt::ControlModifier)
        mSelectionMode = Subtract;
    else if (modifiers == Qt::ShiftModifier)
        mSelectionMode = Add;
    else if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier))
        mSelectionMode = Intersect;
    else
        mSelectionMode = mDefaultMode;

    switch (mSelectionMode) {
    case Replace:   mReplace->setChecked(true); break;
    case Add:       mAdd->setChecked(true); break;
    case Subtract:  mSubtract->setChecked(true); break;
    case Intersect: mIntersect->setChecked(true); break;
    }

    mDragModifier = modifiers == Qt::AltModifier ||
                    modifiers == (Qt::AltModifier | Qt::ControlModifier);

    refreshCursor();
}

void AbstractTileSelectionTool::languageChanged()
{
    mReplace->setToolTip(tr("Replace Selection"));
    mAdd->setToolTip(tr("Add Selection"));
    mSubtract->setToolTip(tr("Subtract Selection"));
    mIntersect->setToolTip(tr("Intersect Selection"));
}

void AbstractTileSelectionTool::populateToolBar(QToolBar *toolBar)
{
    toolBar->addAction(mReplace);
    toolBar->addAction(mAdd);
    toolBar->addAction(mSubtract);
    toolBar->addAction(mIntersect);
}

void AbstractTileSelectionTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    if (mDragModifier) {
        if (mPreviewLayer || !mapDocument()->selectedArea().isEmpty())
            cursorShape = Qt::SizeAllCursor;
    } else if (mPreviewLayer) {
        if (mDragging || brushItem()->tileRegion().contains(tilePosition()))
            cursorShape = Qt::SizeAllCursor;
    }

    if (cursor().shape() != cursorShape)
        setCursor(cursorShape);
}

void AbstractTileSelectionTool::floatSelection(bool duplicate)
{
    const QRegion &selectedArea = mapDocument()->selectedArea();
    if (selectedArea.isEmpty())
        return;

    mTargetLayer = mapDocument()->currentLayer()->asTileLayer();
    mPreviewLayer = SharedTileLayer(mTargetLayer->copy(selectedArea.translated(-mTargetLayer->position())));
    mPreviewLayer->setPosition(selectedArea.boundingRect().topLeft());

    brushItem()->setTileLayer(mPreviewLayer, selectedArea);

    QUndoStack *stack = mapDocument()->undoStack();

    stack->beginMacro(tr("Move Selection"));

    if (!duplicate)
        stack->push(new EraseTiles(mapDocument(), mTargetLayer, selectedArea));

    stack->push(new ChangeSelectedArea(mapDocument(), QRegion()));

    stack->endMacro();
}

void AbstractTileSelectionTool::anchorSelection()
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
