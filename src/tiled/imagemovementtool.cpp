/*
* imagemovementtool.cpp
* Copyright 2014, Mattia Basaglia
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

#include "imagemovementtool.h"
#include "imagelayer.h"
#include "changeimagelayerposition.h"
#include "mapdocument.h"
#include "preferences.h"
#include "maprenderer.h"

#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

ImageMovementTool::ImageMovementTool(QObject *parent) :
    AbstractImageTool(tr("Move Images"),
                       QIcon(QLatin1String(":images/16x16/layer-image.png")),
                       QKeySequence(tr("M")),
                       parent),
    mMousePressed(false)
{
}

void ImageMovementTool::mouseEntered()
{
}

void ImageMovementTool::activate(MapScene *scene)
{
    AbstractImageTool::activate(scene);
}

void ImageMovementTool::deactivate(MapScene *scene)
{
    AbstractImageTool::deactivate(scene);
}

void ImageMovementTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    AbstractImageTool::mouseMoved(pos, modifiers);

    if (mMousePressed) {
        if (ImageLayer *layer = currentImageLayer()) {
            QPointF diff = pos - mMouseStart;

            bool snapToGrid = Preferences::instance()->snapToGrid();
            bool snapToFineGrid = Preferences::instance()->snapToFineGrid();
            if (modifiers & Qt::ControlModifier) {
                snapToGrid = !snapToGrid;
                snapToFineGrid = false;
            }

            if (snapToGrid || snapToFineGrid) {
                MapRenderer *renderer = mapDocument()->renderer();
                int scale = snapToFineGrid ? Preferences::instance()->gridFine() : 1;
                const QPointF alignScreenPos =
                        renderer->pixelToScreenCoords(mLayerStart);
                const QPointF newAlignPixelPos = alignScreenPos + diff;

                // Snap the position to the grid
                QPointF newTileCoords =
                        (renderer->screenToTileCoords(newAlignPixelPos) * scale).toPoint();
                newTileCoords /= scale;
                diff = renderer->tileToScreenCoords(newTileCoords) - alignScreenPos;
            }

            layer->setPosition(mLayerStart+diff.toPoint());
            mapDocument()->emitImageLayerChanged(layer);
        }

    }
}

void ImageMovementTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    mMousePressed = true;
    mMouseStart = event->scenePos();
    ImageLayer *layer = currentImageLayer();
    if (layer) {
        mLayerStart = layer->position();
    }
}

void ImageMovementTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
    mMousePressed = false;

    ImageLayer *layer = currentImageLayer();
    if (layer) {
        QPoint layerFinish = layer->position();
        layer->setPosition(mLayerStart);
        mapDocument()->undoStack()->push(
            new ChangeImageLayerPosition(mapDocument(),
                                         currentImageLayer(),
                                         layerFinish));
    }

}

void ImageMovementTool::languageChanged()
{
    setName(tr("Move Images"));
    setShortcut(QKeySequence(tr("M")));
}
