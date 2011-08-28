/*
 * tileselectiontool.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "changetileselection.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

TileSelectionTool::TileSelectionTool(QObject *parent)
    : AbstractTileTool(tr("Rectangular Select"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-rect-select.png")),
                       QKeySequence(tr("R")),
                       parent)
    , mSelectionMode(Replace)
    , mSelecting(false)
{
    setTilePositionMethod(BetweenTiles);
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

void TileSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    const Qt::MouseButton button = event->button();
    const Qt::KeyboardModifiers modifiers = event->modifiers();

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
        brushItem()->setTileRegion(QRegion());
    }
}

void TileSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mSelecting = false;

        MapDocument *document = mapDocument();
        QRegion selection = document->tileSelection();
        const QRect area = selectedArea();

        switch (mSelectionMode) {
        case Replace:   selection = area; break;
        case Add:       selection += area; break;
        case Subtract:  selection -= area; break;
        case Intersect: selection &= area; break;
        }

        if (selection != document->tileSelection()) {
            QUndoCommand *cmd = new ChangeTileSelection(document, selection);
            document->undoStack()->push(cmd);
        }

        brushItem()->setTileRegion(QRegion());
        updateStatusInfo();
    }
}

void TileSelectionTool::languageChanged()
{
    setName(tr("Rectangular Select"));
    setShortcut(QKeySequence(tr("R")));
}

QRect TileSelectionTool::selectedArea() const
{
    const QPoint tilePos = tilePosition();
    const QPoint pos(qMin(tilePos.x(), mSelectionStart.x()),
                     qMin(tilePos.y(), mSelectionStart.y()));
    const QSize size(qAbs(tilePos.x() - mSelectionStart.x()),
                     qAbs(tilePos.y() - mSelectionStart.y()));

    return QRect(pos, size);
}
