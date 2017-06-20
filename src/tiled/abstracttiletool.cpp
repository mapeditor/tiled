/*
 * abstracttiletool.cpp
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

#include "abstracttiletool.h"

#include "highlighttile.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "tile.h"
#include "tilelayer.h"

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

AbstractTileTool::AbstractTileTool(const QString &name,
                                   const QIcon &icon,
                                   const QKeySequence &shortcut,
                                   QObject *parent)
    : AbstractTool(name, icon, shortcut, parent)
    , mTilePositionMethod(OnTiles)
    , mHighlightTile(new HighlightTile)
    , mHighlightVisible(false)
{
    mHighlightTile->setVisible(false);
    mHighlightTile->setZValue(10000);
}

AbstractTileTool::~AbstractTileTool()
{
    delete mHighlightTile;
}

void AbstractTileTool::activate(MapScene *scene)
{
    scene->addItem(mHighlightTile);
}

void AbstractTileTool::deactivate(MapScene *scene)
{
    scene->removeItem(mHighlightTile);
}

void AbstractTileTool::mouseEntered()
{
    setHighlightVisible(true);
}

void AbstractTileTool::mouseLeft()
{
    setHighlightVisible(false);
}

void AbstractTileTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers)
{
    // Take into account the offset of the current layer
    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer()) {
        offsetPos -= layer->totalOffset();
        mHighlightTile->setLayerOffset(layer->totalOffset());
    }

    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF tilePosF = renderer->screenToTileCoords(offsetPos);
    QPoint tilePos;

    if (mTilePositionMethod == BetweenTiles)
        tilePos = tilePosF.toPoint();
    else
        tilePos = QPoint((int) std::floor(tilePosF.x()),
                         (int) std::floor(tilePosF.y()));

    if (mTilePosition != tilePos) {
        mTilePosition = tilePos;

        tilePositionChanged(tilePos);
        updateStatusInfo();
    }
}

void AbstractTileTool::mapDocumentChanged(MapDocument *oldDocument,
                                          MapDocument *newDocument)
{
    Q_UNUSED(oldDocument)
    mHighlightTile->setMapDocument(newDocument);
}

void AbstractTileTool::updateEnabledState()
{
    setEnabled(currentTileLayer() != nullptr);
}

void AbstractTileTool::updateStatusInfo()
{
    if (mHighlightVisible) {
        int tileId = -1;

        if (const TileLayer *tileLayer = currentTileLayer()) {
            const QPoint pos = tilePosition() - tileLayer->position();
            if (tileLayer->contains(pos))
                tileId = tileLayer->cellAt(pos).tileId();
        }

        QString tileIdString = tileId >= 0 ? QString::number(tileId) : tr("empty");
        setStatusInfo(QString(QLatin1String("%1, %2 [%3]"))
                      .arg(mTilePosition.x())
                      .arg(mTilePosition.y())
                      .arg(tileIdString));
    } else {
        setStatusInfo(QString());
    }
}

void AbstractTileTool::setHighlightVisible(bool visible)
{
    if (mHighlightVisible == visible)
        return;

    mHighlightVisible = visible;
    updateStatusInfo();
    updateHighlightVisibility();
}

void AbstractTileTool::updateHighlightVisibility()
{
    // Show the tile brush only when a visible tile layer is selected
    bool showHighlight = false;
    if (mHighlightVisible) {
        if (Layer *layer = currentTileLayer()) {
            if (layer->isVisible())
                showHighlight = true;
        }
    }
    mHighlightTile->setVisible(showHighlight);
}

TileLayer *AbstractTileTool::currentTileLayer() const
{
    if (!mapDocument())
        return nullptr;

    return dynamic_cast<TileLayer*>(mapDocument()->currentLayer());
}
