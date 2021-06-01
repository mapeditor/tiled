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

#include "brushitem.h"
#include "map.h"
#include "mapdocument.h"

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

    const bool infinite = mapDocument()->map()->infinite();

    QRegion resultRegion;
    if (infinite || tileLayer->contains(tilePos)) {
        const Cell &matchCell = tileLayer->cellAt(tilePos);
        if (matchCell.isEmpty()) {
            // Due to the way TileLayer::region only iterates allocated chunks,
            // and because of different desired behavior for infinite vs. fixed
            // maps we need a special handling when matching the empty cell.
            resultRegion = infinite ? tileLayer->bounds() : tileLayer->rect();
            resultRegion -= tileLayer->region();
        } else {
            resultRegion = tileLayer->region([&] (const Cell &cell) { return cell == matchCell; });
        }
    }
    setSelectedRegion(resultRegion);
    brushItem()->setTileRegion(selectedRegion());
}

void SelectSameTileTool::languageChanged()
{
    setName(tr("Select Same Tile"));

    AbstractTileSelectionTool::languageChanged();
}

#include "moc_selectsametiletool.cpp"
