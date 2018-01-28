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

#include <QApplication>
#include <QKeyEvent>
#include <QUndoStack>

#include <QtMath>

using namespace Tiled;
using namespace Tiled::Internal;

LayerOffsetTool::LayerOffsetTool(QObject *parent)
    : AbstractTool(tr("Offset Layers"),
                   QIcon(QLatin1String(":images/22x22/stock-tool-move-22.png")),
                   QKeySequence(tr("M")),
                   parent)
    , mMousePressed(false)
    , mDragging(false)
    , mApplyingChange(false)
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
    finishDrag();
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
    const int x = qFloor(tilePosF.x());
    const int y = qFloor(tilePosF.y());
    setStatusInfo(QString(QLatin1String("%1, %2")).arg(x).arg(y));

    if (!mMousePressed)
        return;
    if (mApplyingChange)    // avoid recursion
        return;

    if (!mDragging) {
        const QPoint screenPos = QCursor::pos();
        const int dragDistance = (mMouseScreenStart - screenPos).manhattanLength();

        // Use a reduced start drag distance to increase the responsiveness
        if (dragDistance >= QApplication::startDragDistance() / 2)
            startDrag(pos);
    }

    auto currentLayer = mapDocument()->currentLayer();
    if (currentLayer && mDragging) {
        QPointF newOffset = mOldOffset + (pos - mMouseSceneStart);
        SnapHelper(mapDocument()->renderer(), modifiers).snap(newOffset);
        mApplyingChange = true;
        mapDocument()->layerModel()->setLayerOffset(currentLayer, newOffset);
        mApplyingChange = false;
    }
}

void LayerOffsetTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    mMousePressed = true;
    mMouseScreenStart = event->screenPos();
}

void LayerOffsetTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
    mMousePressed = false;
    finishDrag();
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

void LayerOffsetTool::startDrag(const QPointF &pos)
{
    if (!mapDocument())
        return;

    Layer *layer = mapDocument()->currentLayer();
    if (layer && layer->isUnlocked()) {
        mDragging = true;
        mMouseSceneStart = pos;
        mOldOffset = layer->offset();
    }
}

void LayerOffsetTool::finishDrag()
{
    if (!mDragging)
        return;

    mDragging = false;

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
