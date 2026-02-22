/*
 * magicwandtool.cpp
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jared Adams <jaxad0127@gmail.com>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include "magicwandtool.h"

#include "tilepainter.h"

#include "map.h"
#include "mapdocument.h"

#include <algorithm>
#include <QCheckBox>
#include <QToolBar>

using namespace Tiled;

MagicWandTool::MagicWandTool(QObject *parent)
    : AbstractTileSelectionTool("MagicWandTool",
                                tr("Magic Wand"),
                                QIcon(QLatin1String(
                                      ":images/22/stock-tool-fuzzy-select-22.png")),
                                QKeySequence(Qt::Key_W),
                                parent)
{
}

void MagicWandTool::tilePositionChanged(QPoint tilePos)
{
    // Make sure that a tile layer is selected
    TileLayer *tileLayer = currentTileLayer();
    if (!tileLayer)
        return;

    // Reset the list when the left mouse button isn't pressed
    if (!mMouseDown)
        mMatchCells.clear();

    TilePainter tilePainter(mapDocument(), tileLayer);

    const Cell targetCell = tilePainter.cellAt(tilePos);
    if (!mMatchCells.contains(targetCell))
        mMatchCells.append(targetCell);

    const auto condition = [&](const Cell &cell) {
        return mMatchCells.contains(cell);
    };

    if (mContiguous) {
        // Flood fill (default magic wand behavior)
        setSelectionPreview(tilePainter.computeFillRegion(tilePos, condition));
    } else {
        // Select same tile everywhere (like Select Same Tile tool)
        const bool infinite = mapDocument()->map()->infinite();
        const QPoint localPos = tilePos - tileLayer->position();
        QRegion resultRegion;

        if (infinite || tileLayer->contains(localPos)) {
            resultRegion = tileLayer->region(condition);

            // Handle empty cells in the same way as SelectSameTileTool
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
}

void MagicWandTool::languageChanged()
{
    setName(tr("Magic Wand"));

    AbstractTileSelectionTool::languageChanged();
}

void MagicWandTool::populateToolBar(QToolBar *toolBar)
{
    AbstractTileSelectionTool::populateToolBar(toolBar);

    auto contiguousCheckBox = new QCheckBox(tr("Contiguous"), toolBar);
    contiguousCheckBox->setChecked(mContiguous);
    connect(contiguousCheckBox, &QCheckBox::toggled, this, &MagicWandTool::setContiguous);
    toolBar->addWidget(contiguousCheckBox);
}

void MagicWandTool::setContiguous(bool contiguous)
{
    if (mContiguous == contiguous)
        return;

    mContiguous = contiguous;
    mMatchCells.clear();
    tilePositionChanged(tilePosition());
}

#include "moc_magicwandtool.cpp"
