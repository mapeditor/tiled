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

#include "selectiontool.h"

#include "brushitem.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "tilelayer.h"
#include "tileselectionmodel.h"

#include <QGraphicsSceneMouseEvent>

using namespace Tiled;
using namespace Tiled::Internal;

SelectionTool::SelectionTool(QObject *parent)
    : AbstractTool(tr("Rectangular Select"),
                   QIcon(QLatin1String(
                           ":images/22x22/stock-tool-rect-select.png")),
                   parent)
    , mMapScene(0)
    , mBrushItem(new BrushItem)
    , mTileX(0), mTileY(0)
    , mSelectionMode(Replace)
    , mSelecting(false)
    , mBrushVisible(false)
{
    mBrushItem->setVisible(false);
    mBrushItem->setZValue(10000);
    mBrushItem->setTileSize(1, 1);
}

void SelectionTool::enable(MapScene *scene)
{
    mMapScene = scene;

    MapDocument *mapDocument = mMapScene->mapDocument();
    connect(mapDocument, SIGNAL(layerChanged(int)),
            this, SLOT(updateBrushVisibility()));
    connect(mapDocument, SIGNAL(currentLayerChanged(int)),
            this, SLOT(updateBrushVisibility()));

    mBrushItem->setMapDocument(mapDocument);
    mMapScene->addItem(mBrushItem);
    updateBrushVisibility();
}

void SelectionTool::disable()
{
    mMapScene->removeItem(mBrushItem);
    mBrushItem->setMapDocument(0);
    mMapScene = 0;
}

void SelectionTool::enterEvent(QEvent *)
{
    setBrushVisible(true);
}

void SelectionTool::leaveEvent(QEvent *)
{
    setBrushVisible(false);
}

void SelectionTool::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    const Map *map = mMapScene->mapDocument()->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    const QPointF pos = mouseEvent->scenePos();
    const int tileX = ((int) pos.x()) / tileWidth;
    const int tileY = ((int) pos.y()) / tileHeight;

    if (mTileX != tileX || mTileY != tileY) {
        mTileX = tileX;
        mTileY = tileY;
        updatePosition();

        if (mSelecting)
            mBrushItem->setTileSize(selectedArea().size());
    }
}

void SelectionTool::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton) {
        mouseEvent->accept();

        const Qt::KeyboardModifiers modifiers = mouseEvent->modifiers();
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
        mSelectionStart = QPoint(mTileX, mTileY);
        mBrushItem->setTileSize(1, 1);
    }
}

void SelectionTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton) {
        mouseEvent->accept();
        mSelecting = false;

        const MapDocument *mapDocument = mMapScene->mapDocument();
        TileSelectionModel *selectionModel = mapDocument->selectionModel();
        const QRect selection = selectedArea();

        switch (mSelectionMode) {
        case Replace:
            selectionModel->setSelection(selection);
            break;
        case Add:
            selectionModel->addRect(selection);
            break;
        case Subtract:
            selectionModel->subtractRect(selection);
            break;
        case Intersect:
            selectionModel->intersectRect(selection);
            break;
        }

        mBrushItem->setTileSize(1, 1);
        updatePosition();
    }
}

/* TODO: The methods below are copies of those in StampBrush. They should be in
 *       a baseclass.
 */

QRect SelectionTool::selectedArea() const
{
    QRect selected = QRect(mSelectionStart,
                           QPoint(mTileX, mTileY)).normalized();
    if (selected.width() == 0)
        selected.adjust(-1, 0, 1, 0);
    if (selected.height() == 0)
        selected.adjust(0, -1, 0, 1);
    return selected;
}

/**
 * Updates the position of the brush item.
 */
void SelectionTool::updatePosition()
{
    QPoint newPos;

    if (mSelecting) {
        newPos = QPoint(qMin(mTileX, mSelectionStart.x()),
                        qMin(mTileY, mSelectionStart.y()));
    } else {
        newPos = QPoint(mTileX, mTileY);
    }

    mBrushItem->setTilePos(newPos);
}

void SelectionTool::setBrushVisible(bool visible)
{
    if (mBrushVisible == visible)
        return;

    mBrushVisible = visible;
    updateBrushVisibility();
}

void SelectionTool::updateBrushVisibility()
{
    // Show the tile brush only when a visible tile layer is selected
    bool showBrush = false;
    if (mBrushVisible) {
        if (Layer *layer = currentTileLayer()) {
            if (layer->isVisible())
                showBrush = true;
        }
    }
    mBrushItem->setVisible(showBrush);
}

/**
 * Returns the current tile layer, or 0 if no tile layer is current selected.
 */
TileLayer *SelectionTool::currentTileLayer() const
{
    if (!mMapScene)
        return 0;

    MapDocument *mapDocument = mMapScene->mapDocument();
    const int currentLayerIndex = mapDocument->currentLayer();
    if (currentLayerIndex < 0)
        return 0;
    Layer *currentLayer = mapDocument->map()->layerAt(currentLayerIndex);
    return dynamic_cast<TileLayer*>(currentLayer);
}
