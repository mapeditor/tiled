/*
 * tileselectionitem.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tileselectionitem.h"

#include "grouplayer.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QStyleOptionGraphicsItem>

using namespace Tiled;
using namespace Tiled::Internal;

TileSelectionItem::TileSelectionItem(MapDocument *mapDocument,
                                     QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mMapDocument(mapDocument)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);

    connect(mapDocument, &MapDocument::selectedAreaChanged,
            this, &TileSelectionItem::selectionChanged);
    connect(mapDocument, &MapDocument::layerChanged,
            this, &TileSelectionItem::layerChanged);
    connect(mapDocument, &MapDocument::currentLayerChanged,
            this, &TileSelectionItem::currentLayerChanged);

    updateBoundingRect();
}

QRectF TileSelectionItem::boundingRect() const
{
    return mBoundingRect;
}

void TileSelectionItem::paint(QPainter *painter,
                              const QStyleOptionGraphicsItem *option,
                              QWidget *)
{
    const QRegion &selection = mMapDocument->selectedArea();
    QColor highlight = QApplication::palette().highlight().color();
    highlight.setAlpha(128);

    MapRenderer *renderer = mMapDocument->renderer();
    renderer->drawTileSelection(painter, selection, highlight,
                                option->exposedRect);
}

void TileSelectionItem::selectionChanged(const QRegion &newSelection,
                                         const QRegion &oldSelection)
{
    prepareGeometryChange();
    updateBoundingRect();

    // Make sure changes within the bounding rect are updated
    const QRect changedArea = newSelection.xored(oldSelection).boundingRect();
    update(mMapDocument->renderer()->boundingRect(changedArea));
}

void TileSelectionItem::layerChanged(Layer *layer)
{
    if (auto currentLayer = mMapDocument->currentLayer())
        if (currentLayer->isParentOrSelf(layer))
            setPos(currentLayer->totalOffset());
}

void TileSelectionItem::currentLayerChanged(Layer *layer)
{
    if (layer)
        setPos(layer->totalOffset());
}

void TileSelectionItem::updateBoundingRect()
{
    const QRect b = mMapDocument->selectedArea().boundingRect();
    mBoundingRect = mMapDocument->renderer()->boundingRect(b);
}
