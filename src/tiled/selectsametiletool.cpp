/*
 * selectsametiletool.cpp
 * Copyright 2015, Mamed Ibrahimov <ibramlab@gmail.com>
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

#include "selectsametiletool.h"

#include "map.h"
#include "mapdocument.h"

#include <QGraphicsSceneMouseEvent>

using namespace Tiled;

SelectSameTileTool::SelectSameTileTool(QObject *parent)
    : AbstractTileSelectionTool("SelectSameTileTool",
                                tr("Select Same Tile"),
                                QIcon(QLatin1String(
                                      ":images/22/stock-tool-by-color-select.png")),
                                QKeySequence(Qt::Key_S),
                                parent)
{
}

void SelectSameTileTool::tilePositionChanged(QPoint tilePos)
{
    // Make sure that a tile layer is selected and contains current tile pos.
    TileLayer *tileLayer = currentTileLayer();
    if (!tileLayer)
        return;

    // Reset the list when the left mouse button isn't pressed
    if (!mMouseDown)
        mMatchCells.clear();

    const bool infinite = mapDocument()->map()->infinite();
    QRegion resultRegion;

    if (infinite || tileLayer->contains(tilePos)) {
        const Cell &currentCell = tileLayer->cellAt(tilePos);
        if (!mMatchCells.contains(currentCell))
            mMatchCells.append(currentCell);

        resultRegion = tileLayer->region(
            [&](const Cell &cell) { return mMatchCells.contains(cell); });

        // Due to the way TileLayer::region only iterates allocated chunks, and
        // because of different desired behavior for infinite vs. fixed maps we
        // need a special handling when matching the empty cell.
        const bool hasEmptyCell = std::any_of(mMatchCells.begin(),
                                              mMatchCells.end(),
                                              [](const Cell &cell) { return cell.isEmpty(); });
        if (hasEmptyCell) {
            QRegion emptyRegion = infinite ? tileLayer->bounds() : tileLayer->rect();
            emptyRegion -= tileLayer->region();

            resultRegion += emptyRegion;
        }
    }

    setSelectionPreview(resultRegion);
}

void SelectSameTileTool::languageChanged()
{
    setName(tr("Select Same Tile"));

    AbstractTileSelectionTool::languageChanged();
}

#include "moc_selectsametiletool.cpp"
