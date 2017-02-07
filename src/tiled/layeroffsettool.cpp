/*
* layeroffsettool.cpp
* Copyright 2014, Mattia Basaglia
* Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "layeroffsettool.h"

#include "changelayer.h"
#include "layermodel.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "snaphelper.h"

#include <QUndoStack>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

LayerOffsetTool::LayerOffsetTool(QObject *parent) :
    AbstractTool(tr("Offset Layers"),
                 QIcon(QLatin1String(":images/22x22/stock-tool-move-22.png")),
                 QKeySequence(tr("M")),
                 parent),
    mMousePressed(false),
    mApplyingChange(false)
{
}

void LayerOffsetTool::mouseEntered()
{
}

void LayerOffsetTool::mouseLeft()
{
}

void LayerOffsetTool::activate(MapScene *)
{
}

void LayerOffsetTool::deactivate(MapScene *)
{
}

void LayerOffsetTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (!mapDocument())
        return;

    // Take into account the offset of the current layer
    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer())
        offsetPos -= layer->totalOffset();

    const QPointF tilePosF = mapDocument()->renderer()->screenToTileCoords(offsetPos);
    const int x = (int) std::floor(tilePosF.x());
    const int y = (int) std::floor(tilePosF.y());
    setStatusInfo(QString(QLatin1String("%1, %2")).arg(x).arg(y));

    if (!mMousePressed)
        return;
    if (mApplyingChange)    // avoid recursion
        return;

    auto currentLayer = mapDocument()->currentLayer();

    if (currentLayer) {
        QPointF newOffset = mOldOffset + (pos - mMouseStart);
        SnapHelper(mapDocument()->renderer(), modifiers).snap(newOffset);
        mApplyingChange = true;
        mapDocument()->layerModel()->setLayerOffset(currentLayer, newOffset);
        mApplyingChange = false;
    }
}

void LayerOffsetTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    mMousePressed = true;
    mMouseStart = event->scenePos();

    if (!mapDocument())
        return;

    if (Layer *layer = mapDocument()->currentLayer())
        mOldOffset = layer->offset();
}

void LayerOffsetTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
    mMousePressed = false;

    if (!mapDocument())
        return;

    if (Layer *layer = mapDocument()->currentLayer()) {
        const QPointF newOffset = layer->offset();
        auto currentLayer = mapDocument()->currentLayer();
        mApplyingChange = true;
        mapDocument()->layerModel()->setLayerOffset(currentLayer, mOldOffset);
        mapDocument()->undoStack()->push(
                    new SetLayerOffset(mapDocument(),
                                       currentLayer,
                                       newOffset));
        mApplyingChange = false;
    }
}

void LayerOffsetTool::modifiersChanged(Qt::KeyboardModifiers)
{
}

void LayerOffsetTool::languageChanged()
{
    setName(tr("Offset Layers"));
    setShortcut(QKeySequence(tr("M")));
}

void LayerOffsetTool::updateEnabledState()
{
    setEnabled(mapDocument() && mapDocument()->currentLayer());
}
