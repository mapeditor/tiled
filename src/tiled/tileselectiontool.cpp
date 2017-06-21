/*
 * tileselectiontool.cpp
 * Copyright 2009-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tileselectiontool.h"

#include "brushitem.h"
#include "changeselectedarea.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "tilelayer.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QToolBar>

using namespace Tiled;
using namespace Tiled::Internal;

TileSelectionTool::TileSelectionTool(QObject *parent)
    : AbstractTileTool(tr("Rectangular Select"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-rect-select.png")),
                       QKeySequence(tr("R")),
                       parent)
    , mSelectionMode(Replace)
    , mMouseDown(false)
    , mSelecting(false)
{
    setTilePositionMethod(OnTiles);
    QIcon replaceIcon(QLatin1String(":images/16x16/selection-replace.png"));
    QIcon addIcon(QLatin1String(":images/16x16/selection-add.png"));
    QIcon subtractIcon(QLatin1String(":images/16x16/selection-subtract.png"));
    QIcon intersectIcon(QLatin1String(":images/16x16/selection-intersect.png"));

    mReplace = new QAction(this);
    mReplace->setIcon(replaceIcon);
    mReplace->setCheckable(true);
    mReplace->setChecked(true);
    mReplace->setToolTip(tr("Replace Selection"));

    mAdd = new QAction(this);
    mAdd->setIcon(addIcon);
    mAdd->setCheckable(true);
    mAdd->setToolTip(tr("Add Selection"));
    mReplace->setShortcut(QKeySequence(tr("Ctrl")));

    mSubtract = new QAction(this);
    mSubtract->setIcon(subtractIcon);
    mSubtract->setCheckable(true);
    mSubtract->setToolTip(tr("Subtract Selection"));

    mIntersect = new QAction(this);
    mIntersect->setIcon(intersectIcon);
    mIntersect->setCheckable(true);
    mIntersect->setToolTip(tr("Intersect Selection"));

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
}

void TileSelectionTool::tilePositionChanged(const QPoint &)
{
    if (mSelecting)
        brushItem()->setTileRegion(selectedArea());
}

void TileSelectionTool::updateStatusInfo()
{
    if (!isBrushVisible() || !mSelecting) {
        AbstractTileTool::updateStatusInfo();
        return;
    }

    const QPoint pos = tilePosition();
    const QRect area = selectedArea();
    setStatusInfo(tr("%1, %2 - Rectangle: (%3 x %4)")
                  .arg(pos.x()).arg(pos.y())
                  .arg(area.width()).arg(area.height()));
}

void TileSelectionTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (mMouseDown && !mSelecting) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mMouseScreenStart - screenPos).manhattanLength();

        // Use a reduced start drag distance to increase the responsiveness
        if (dragDistance >= QApplication::startDragDistance() / 2) {
            mSelecting = true;
            tilePositionChanged(tilePosition());
        }
    }

    AbstractTileTool::mouseMoved(pos, modifiers);
}

void TileSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    const Qt::MouseButton button = event->button();

    if (button == Qt::LeftButton) {
        mMouseDown = true;
        mMouseScreenStart = event->screenPos();
        mSelectionStart = tilePosition();
        brushItem()->setTileRegion(QRegion());
    }

    if (button == Qt::RightButton) {
        if (mSelecting) {
            // Cancel selecting
            mSelecting = false;
            mMouseDown = false; // Avoid restarting select on move
            brushItem()->setTileRegion(QRegion());
        } else {
            clearSelection();
        }
    }
}

void TileSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (mSelecting) {
        mSelecting = false;

        MapDocument *document = mapDocument();
        QRegion selection = document->selectedArea();
        const QRect area = selectedArea();

        switch (mSelectionMode) {
        case Replace:   selection = area; break;
        case Add:       selection += area; break;
        case Subtract:  selection -= area; break;
        case Intersect: selection &= area; break;
        }

        if (selection != document->selectedArea()) {
            QUndoCommand *cmd = new ChangeSelectedArea(document, selection);
            document->undoStack()->push(cmd);
        }

        brushItem()->setTileRegion(QRegion());
        updateStatusInfo();
    } else if (mMouseDown) {
        // Clicked without dragging and not cancelled
        clearSelection();
    }

    mMouseDown = false;
}

void TileSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if (modifiers == Qt::ControlModifier) {
        mSelectionMode = Subtract;
        mSubtract->setChecked(true);
    } else if (modifiers == Qt::ShiftModifier) {
        mSelectionMode = Add;
        mAdd->setChecked(true);
    } else if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
        mSelectionMode = Intersect;
        mIntersect->setChecked(true);
    } else {
        mSelectionMode = mDefaultMode;
        switch (mDefaultMode) {
        case Replace:   mReplace->setChecked(true); break;
        case Add:       mAdd->setChecked(true); break;
        case Subtract:  mSubtract->setChecked(true); break;
        case Intersect: mIntersect->setChecked(true); break;
        }
    }
}

void TileSelectionTool::languageChanged()
{
    setName(tr("Rectangular Select"));
    setShortcut(QKeySequence(tr("R")));
    mReplace->setToolTip(tr("Replace Selection"));
    mAdd->setToolTip(tr("Add Selection"));
    mSubtract->setToolTip(tr("Subtract Selection"));
    mIntersect->setToolTip(tr("Intersect Selection"));
}

void TileSelectionTool::populateToolBar(QToolBar *toolBar)
{
    toolBar->addAction(mReplace);
    toolBar->addAction(mAdd);
    toolBar->addAction(mSubtract);
    toolBar->addAction(mIntersect);
}

QRect TileSelectionTool::selectedArea() const
{
    QRect area = QRect(mSelectionStart, tilePosition()).normalized();
    if (area.width() == 0)
        area.adjust(-1, 0, 1, 0);
    if (area.height() == 0)
        area.adjust(0, -1, 0, 1);
    return area;
}

void TileSelectionTool::clearSelection()
{
    MapDocument *document = mapDocument();
    if (!document->selectedArea().isEmpty()) {
        QUndoCommand *cmd = new ChangeSelectedArea(document, QRegion());
        document->undoStack()->push(cmd);
    }
}
