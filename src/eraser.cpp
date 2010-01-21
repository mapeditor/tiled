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

#include "eraser.h"

#include "brushitem.h"
#include "erasetiles.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

Eraser::Eraser(QObject *parent)
    : AbstractTileTool(tr("Eraser"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-eraser.png")),
                       QKeySequence(tr("E")),
                       parent)
    , mErasing(false)
{
}

void Eraser::tilePositionChanged(const QPoint &tilePos)
{
    brushItem()->setTileRegion(QRect(tilePos, QSize(1, 1)));

    if (mErasing)
        doErase(true);
}

void Eraser::mousePressed(const QPointF &, Qt::MouseButton button,
                          Qt::KeyboardModifiers)
{
    if (button == Qt::LeftButton) {
        mErasing = true;
        doErase(false);
    }
}

void Eraser::mouseReleased(const QPointF &, Qt::MouseButton button)
{
    if (button == Qt::LeftButton)
        mErasing = false;
}

void Eraser::languageChanged()
{
    setName(tr("Eraser"));
    setShortcut(QKeySequence(tr("E")));
}

void Eraser::doErase(bool mergeable)
{
    MapDocument *mapDocument = mapScene()->mapDocument();
    TileLayer *tileLayer = currentTileLayer();
    const QPoint tilePos = tilePosition();

    if (!tileLayer->bounds().contains(tilePos))
        return;

    EraseTiles *erase = new EraseTiles(mapDocument, tileLayer,
                                       QRegion(tilePos.x(),
                                               tilePos.y(),
                                               1, 1));
    erase->setMergeable(mergeable);

    mapDocument->undoStack()->push(erase);
}
