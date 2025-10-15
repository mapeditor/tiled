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
#include "mapdocument.h"
#include "mapscene.h"

#include <QApplication>
#include <QKeyEvent>

using namespace Tiled;

TileSelectionTool::TileSelectionTool(QObject *parent)
    : AbstractTileSelectionTool("TileSelectionTool",
                                tr("Rectangular Select"),
                                QIcon(QLatin1String(
                                      ":images/22/stock-tool-rect-select.png")),
                                QKeySequence(Qt::Key_R),
                                parent)
{
    setTilePositionMethod(OnTiles);
}

void TileSelectionTool::tilePositionChanged(QPoint)
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
        mForceSquare = false;
        mExpandFromCenter = false;
        mMouseScreenStart = event->screenPos();
        mSelectionStart = tilePosition();
        brushItem()->setTileRegion(QRegion());
        return;
    }

    if (button == Qt::RightButton) {
        if (mSelecting) {
            // Cancel selecting
            mSelecting = false;
            mMouseDown = false; // Avoid restarting select on move
            brushItem()->setTileRegion(QRegion());
            return;
        }

        if (event->modifiers() == Qt::NoModifier) {
            clearSelection();
            return;
        }
    }

    AbstractTileTool::mousePressed(event);  // skipping AbstractTileSelection on purpose
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

        switch (selectionMode()) {
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
    if (mMouseDown) {
        // When the mouse is down, we no longer change the selection mode. Instead:
        //
        // * The Shift modifier can be used to force a 1:1 aspect ratio
        // * The Control modifier can be used to expand from the center
        //
        // Only a change in modifier state has an effect because we don't want
        // modifiers that were already held when starting the selection to
        // affect these options, since at that point they were used to set the
        // selection mode.

        const bool shift = modifiers & Qt::ShiftModifier;
        const bool ctrl = modifiers & Qt::ControlModifier;

        if (shift != (mLastModifiers & Qt::ShiftModifier))
            mForceSquare = shift;

        if (ctrl != (mLastModifiers & Qt::ControlModifier))
            mExpandFromCenter = ctrl;

        tilePositionChanged(tilePosition());
        updateStatusInfo();

    } else {
        AbstractTileSelectionTool::modifiersChanged(modifiers);
    }

    mLastModifiers = modifiers;
}

void TileSelectionTool::keyPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (mSelecting) {
            // Cancel the ongoing selection
            mSelecting = false;
            mMouseDown = false;
            brushItem()->setTileRegion(QRegion());
            updateStatusInfo();
            return;
        }
    }

    AbstractTileSelectionTool::keyPressed(event);
}

void TileSelectionTool::languageChanged()
{
    setName(tr("Rectangular Select"));

    AbstractTileSelectionTool::languageChanged();
}

QRect TileSelectionTool::selectedArea() const
{
    QPoint startPos = mSelectionStart;
    QPoint endPos = tilePosition();
    QPoint delta = endPos - startPos;

    if (mForceSquare) {
        const int size = qMax(qAbs(delta.x()), qAbs(delta.y()));
        delta.setX((delta.x() < 0) ? -size : size);
        delta.setY((delta.y() < 0) ? -size : size);
        endPos = startPos + delta;
    }

    if (mExpandFromCenter)
        startPos -= delta;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRect area = QRect(startPos, endPos).normalized();
    if (area.width() == 0)
        area.adjust(-1, 0, 1, 0);
    if (area.height() == 0)
        area.adjust(0, -1, 0, 1);
    return area;
#else
    return QRect::span(startPos, endPos);
#endif
}

void TileSelectionTool::clearSelection()
{
    MapDocument *document = mapDocument();
    if (!document->selectedArea().isEmpty()) {
        QUndoCommand *cmd = new ChangeSelectedArea(document, QRegion());
        document->undoStack()->push(cmd);
    }
}

#include "moc_tileselectiontool.cpp"
