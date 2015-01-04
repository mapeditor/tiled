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
#include "filltiles.h"
#include "tilepainter.h"
#include "tile.h"
#include "mapscene.h"
#include "mapdocument.h"
#include "changeselectedarea.h"

#include <QApplication>

using namespace Tiled;
using namespace Tiled::Internal;

MagicWandTool::MagicWandTool(QObject *parent)
    : AbstractTileTool(tr("Bucket Fill Tool"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-bucket-fill.png")),
                       QKeySequence(tr("F")),
                       parent)
{
}

/*void MagicWandTool::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);

    mIsActive = true;
    tilePositionChanged(tilePosition());
}

void MagicWandTool::deactivate(MapScene *scene)
{
    AbstractTileTool::deactivate(scene);

    mFillRegion = QRegion();
    mIsActive = false;
}*/

void MagicWandTool::tilePositionChanged(const QPoint &tilePos)
{
    bool shiftPressed = QApplication::keyboardModifiers() & Qt::ShiftModifier;

    // Make sure that a tile layer is selected
    TileLayer *tileLayer = currentTileLayer();
    if (!tileLayer)
        return;

    TilePainter regionComputer(mapDocument(), tileLayer);

    // Get the new select region
    if (!shiftPressed) {
        // If not holding shift, a region is generated from the current pos
        mSelectedRegion = regionComputer.computeFillRegion(tilePos);
    }// else {

    brushItem()->setTileRegion(mSelectedRegion);
}

void MagicWandTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    MapDocument *document = mapDocument();

    if (mSelectedRegion != document->selectedArea()) {
        QUndoCommand *cmd = new ChangeSelectedArea(document, mSelectedRegion);
        document->undoStack()->push(cmd);
    }
}

void MagicWandTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
}

void MagicWandTool::languageChanged()
{
    setName(tr("Magic Wand"));
    //setShortcut(QKeySequence(tr("W")));
    // TODO: Select a suitable shortcut.
}
