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
                       parent)
    , mSelectionMode(Replace)
    , mSelecting(false)
{
}

void SelectionTool::tilePositionChanged(const QPoint &)
{
    updatePosition();

    if (mSelecting)
        brushItem()->setTileSize(selectedArea().size());
}

void SelectionTool::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton) {
        mouseEvent->accept();

        const Qt::KeyboardModifiers modifiers = mouseEvent->modifiers();
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
        brushItem()->setTileSize(1, 1);
    }
}

void SelectionTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton) {
        mouseEvent->accept();
        mSelecting = false;

        const MapDocument *mapDocument = mapScene()->mapDocument();
        TileSelectionModel *selectionModel = mapDocument->selectionModel();
        const QRect selection = selectedArea();

        switch (mSelectionMode) {
        case Replace:
            selectionModel->setSelection(selection);
            break;
        case Add:
            selectionModel->addRect(selection);
            break;
        case Subtract:
            selectionModel->subtractRect(selection);
            break;
        case Intersect:
            selectionModel->intersectRect(selection);
            break;
        }

        brushItem()->setTileSize(1, 1);
        updatePosition();
    }
}

QRect SelectionTool::selectedArea() const
{
    QRect selected = QRect(mSelectionStart, tilePosition()).normalized();
    if (selected.width() == 0)
        selected.adjust(-1, 0, 1, 0);
    if (selected.height() == 0)
        selected.adjust(0, -1, 0, 1);
    return selected;
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
