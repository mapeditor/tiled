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

#include "brushitem.h"
#include "map.h"
#include "mapdocument.h"
#include "tilepainter.h"

#include <QAction>
#include <QToolbar>

using namespace Tiled;
using namespace Tiled::Internal;

MagicWandTool::MagicWandTool(QObject *parent)
    : AbstractTileSelectionTool(tr("Magic Wand"),
                                QIcon(QLatin1String(
                                      ":images/22x22/stock-tool-fuzzy-select-22.png")),
                                QKeySequence(tr("W")),
                                parent),
      mScope(Contiguous)
{
    QIcon contiguousScopeIcon(QLatin1String(":images/22x22/stock-tool-fuzzy-select-22.png"));
    QIcon globalScopeIcon(QLatin1String(":images/22x22/stock-tool-by-color-select.png"));

    mContiguousScope = new QAction(this);
    mContiguousScope->setIcon(contiguousScopeIcon);
    mContiguousScope->setCheckable(true);
    mContiguousScope->setChecked(true);

    mGlobalScope = new QAction(this);
    mGlobalScope->setIcon(globalScopeIcon);
    mGlobalScope->setCheckable(true);
    mGlobalScope->setChecked(false);

    mScopeActionGroup = new QActionGroup(this);
    mScopeActionGroup->addAction(mContiguousScope);
    mScopeActionGroup->addAction(mGlobalScope);

    connect(mContiguousScope, &QAction::triggered,
            [this]() { mScope = Contiguous; });
    connect(mGlobalScope, &QAction::triggered,
            [this]() { mScope = Global; });
}

void MagicWandTool::tilePositionChanged(const QPoint &tilePos)
{
    // Make sure that a tile layer is selected
    TileLayer *tileLayer = currentTileLayer();
    if (!tileLayer)
        return;

    QRegion resultRegion;
    if (mScope == Contiguous) {
        TilePainter regionComputer(mapDocument(), tileLayer);
        resultRegion = regionComputer.computeFillRegion(tilePos);
    } else if (mScope == Global) {
        if (mapDocument()->map()->infinite() || tileLayer->contains(tilePos)) {
            const Cell &matchCell = tileLayer->cellAt(tilePos);
            resultRegion = tileLayer->region([&] (const Cell &cell) { return cell == matchCell; });
        }
    }

    setSelectedRegion(resultRegion);
    brushItem()->setTileRegion(selectedRegion());
}

void MagicWandTool::languageChanged()
{
    setName(tr("Magic Wand"));
    setShortcut(QKeySequence(tr("W")));

    AbstractTileSelectionTool::languageChanged();
}

void MagicWandTool::populateToolBar(QToolBar *toolBar)
{
    AbstractTileSelectionTool::populateToolBar(toolBar);
    toolBar->addSeparator();
    toolBar->addAction(mContiguousScope);
    toolBar->addAction(mGlobalScope);
}
