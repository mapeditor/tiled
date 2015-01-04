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

}

void MagicWandTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    MapDocument *document = mapDocument();

    if (mSelectedRegion != document->selectedArea()) {
        QUndoCommand *cmd = new ChangeSelectedArea(document, mSelectedRegion);
        document->undoStack()->push(cmd);
    }

    brushItem()->setTileRegion(mSelectedRegion);
    document->setSelectedArea(mSelectedRegion);
}

void MagicWandTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
}

/*void MagicWandTool::modifiersChanged(Qt::KeyboardModifiers)
{
    // Don't need to recalculate fill region if there was no fill region
    if (!mFillOverlay)
        return;

    tilePositionChanged(tilePosition());
}
*/
void MagicWandTool::languageChanged()
{
    setName(tr("Magic Wand"));
    //setShortcut(QKeySequence(tr("W")));
    // TODO: Select a suitable shortcut.
}
/*
void MagicWandTool::mapDocumentChanged(MapDocument *oldDocument,
                                        MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    clearConnections(oldDocument);

    // Reset things that are probably invalid now
    setStamp(0);
    clearOverlay();
}

void MagicWandTool::setStamp(TileLayer *stamp)
{
    // Clear any overlay that we presently have with an old stamp
    clearOverlay();

    delete mStamp;
    mStamp = stamp;

    if (mIsRandom)
        updateRandomList();

    if (mIsActive && brushItem()->isVisible())
        tilePositionChanged(tilePosition());
}

void MagicWandTool::clearOverlay()
{
    // Clear connections before clearing overlay so there is no
    // risk of getting a callback and causing an infinite loop
    clearConnections(mapDocument());

    brushItem()->setTileLayer(0);
    delete mFillOverlay;
    mFillOverlay = 0;

    mFillRegion = QRegion();
    brushItem()->setTileRegion(QRegion());
}

void MagicWandTool::makeConnections()
{
    if (!mapDocument())
        return;

    // Overlay may need to be cleared if a region changed
    connect(mapDocument(), SIGNAL(regionChanged(QRegion)),
            this, SLOT(clearOverlay()));

    // Overlay needs to be cleared if we switch to another layer
    connect(mapDocument(), SIGNAL(currentLayerIndexChanged(int)),
            this, SLOT(clearOverlay()));

    // Overlay needs be cleared if the selection changes, since
    // the overlay may be bound or may need to be bound to the selection
    connect(mapDocument(), SIGNAL(selectedAreaChanged(QRegion,QRegion)),
            this, SLOT(clearOverlay()));
}

void MagicWandTool::clearConnections(MapDocument *mapDocument)
{
    if (!mapDocument)
        return;

    disconnect(mapDocument, SIGNAL(regionChanged(QRegion)),
               this, SLOT(clearOverlay()));

    disconnect(mapDocument, SIGNAL(currentLayerIndexChanged(int)),
               this, SLOT(clearOverlay()));

    disconnect(mapDocument, SIGNAL(selectedAreaChanged(QRegion,QRegion)),
               this, SLOT(clearOverlay()));
}

void MagicWandTool::setRandom(bool value)
{
    if (mIsRandom == value)
        return;

    mIsRandom = value;

    if (mIsRandom)
        updateRandomList();
    else
        mRandomList.clear();

    // Don't need to recalculate fill region if there was no fill region
    if (!mFillOverlay)
        return;

    tilePositionChanged(tilePosition());
}

TileLayer *MagicWandTool::getRandomTileLayer(const QRegion &region) const
{
    QRect bb = region.boundingRect();
    TileLayer *result = new TileLayer(QString(), bb.x(), bb.y(),
                                      bb.width(), bb.height());

    if (region.isEmpty() || mRandomList.empty())
        return result;

    foreach (const QRect &rect, region.rects()) {
        for (int _x = rect.left(); _x <= rect.right(); ++_x) {
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y) {

                result->setCell(_x - bb.x(),
                                _y - bb.y(),
                                mRandomList.at(rand() % mRandomList.size()));
            }
        }
    }
    return result;
}

void MagicWandTool::updateRandomList()
{
    mRandomList.clear();

    if (!mStamp)
        return;

    for (int x = 0; x < mStamp->width(); x++)
        for (int y = 0; y < mStamp->height(); y++)
            if (!mStamp->cellAt(x, y).isEmpty())
                mRandomList.append(mStamp->cellAt(x, y));
}
*/
