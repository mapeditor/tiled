/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "selectiontool.h"

#include "brushitem.h"
#include "changeselection.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "tilelayer.h"
#include "tileselectionmodel.h"

#include <QGraphicsSceneMouseEvent>

using namespace Tiled;
using namespace Tiled::Internal;

SelectionTool::SelectionTool(QObject *parent)
    : AbstractTileTool(tr("Rectangular Select"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-rect-select.png")),
                       QKeySequence(tr("R")),
                       parent)
    , mSelectionMode(Replace)
    , mSelecting(false)
{
    setTilePositionMethod(BetweenTiles);
    brushItem()->setTileSize(0, 0);
}

void SelectionTool::tilePositionChanged(const QPoint &)
{
    updatePosition();

    if (mSelecting)
        brushItem()->setTileSize(selectedArea().size());
}

void SelectionTool::updateStatusInfo()
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

void SelectionTool::mousePressed(const QPointF &, Qt::MouseButton button,
                                 Qt::KeyboardModifiers modifiers)
{
    if (button == Qt::LeftButton) {
        if (modifiers == Qt::ControlModifier) {
            mSelectionMode = Subtract;
        } else if (modifiers == Qt::ShiftModifier) {
            mSelectionMode = Add;
        } else if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
            mSelectionMode = Intersect;
        } else {
            mSelectionMode = Replace;
        }

        mSelecting = true;
        mSelectionStart = tilePosition();
        brushItem()->setTileSize(0, 0);
    }
}

void SelectionTool::mouseReleased(const QPointF &,
                                  Qt::MouseButton button)
{
    if (button == Qt::LeftButton) {
        mSelecting = false;

        MapDocument *mapDocument = mapScene()->mapDocument();
        TileSelectionModel *selectionModel = mapDocument->selectionModel();
        QRegion selection = selectionModel->selection();
        const QRect area = selectedArea();

        switch (mSelectionMode) {
        case Replace:   selection = area; break;
        case Add:       selection += area; break;
        case Subtract:  selection -= area; break;
        case Intersect: selection &= area; break;
        }

        if (selection != selectionModel->selection()) {
            QUndoCommand *cmd = new ChangeSelection(mapDocument, selection);
            mapDocument->undoStack()->push(cmd);
        }

        brushItem()->setTileSize(0, 0);
        updatePosition();
        updateStatusInfo();
    }
}

QRect SelectionTool::selectedArea() const
{
    const QPoint tilePos = tilePosition();
    const QPoint pos(qMin(tilePos.x(), mSelectionStart.x()),
                     qMin(tilePos.y(), mSelectionStart.y()));
    const QSize size(qAbs(tilePos.x() - mSelectionStart.x()),
                     qAbs(tilePos.y() - mSelectionStart.y()));

    return QRect(pos, size);
}

/**
 * Updates the position of the brush item.
 */
void SelectionTool::updatePosition()
{
    const QPoint tilePos = tilePosition();
    QPoint newPos;

    if (mSelecting) {
        newPos = QPoint(qMin(tilePos.x(), mSelectionStart.x()),
                        qMin(tilePos.y(), mSelectionStart.y()));
    } else {
        newPos = tilePos;
    }

    brushItem()->setTilePos(newPos);
}
