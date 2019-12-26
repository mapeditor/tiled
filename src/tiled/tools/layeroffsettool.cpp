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
#include "grouplayer.h"
#include "layermodel.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "snaphelper.h"

#include <QApplication>
#include <QKeyEvent>
#include <QUndoStack>

#include <QtMath>

#include "qtcompat_p.h"

using namespace Tiled;

LayerOffsetTool::LayerOffsetTool(QObject *parent)
    : AbstractTool("LayerOffsetTool",
                   tr("Offset Layers"),
                   QIcon(QLatin1String(":images/22/stock-tool-move-22.png")),
                   QKeySequence(Qt::Key_M),
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

void LayerOffsetTool::keyPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        abortDrag();
        return;
    }

    event->ignore();
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

    mApplyingChange = true;
    for (const DraggingLayer &dragging : qAsConst(mDraggingLayers)) {
        QPointF newOffset = dragging.oldOffset + (pos - mMouseSceneStart);
        SnapHelper(mapDocument()->renderer(), modifiers).snap(newOffset);
        dragging.layer->setOffset(newOffset);
        emit mapDocument()->changed(LayerChangeEvent(dragging.layer, LayerChangeEvent::OffsetProperty));
    }
    mApplyingChange = false;
}

void LayerOffsetTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        abortDrag();
    } else if (event->button() == Qt::LeftButton) {
        mMousePressed = true;
        mMouseScreenStart = event->screenPos();
        setCursor(Qt::SizeAllCursor);
    }
}

void LayerOffsetTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        finishDrag();
}

void LayerOffsetTool::modifiersChanged(Qt::KeyboardModifiers)
{
}

void LayerOffsetTool::languageChanged()
{
    setName(tr("Offset Layers"));
}

void LayerOffsetTool::updateEnabledState()
{
    setEnabled(currentLayer());
}

void LayerOffsetTool::mapDocumentChanged(MapDocument *oldDocument,
                                         MapDocument *newDocument)
{
    // No changes should be happening while dragging, but there is currently
    // no mechanism to stop it. Abort drag if any layers are going to be
    // removed, to avoid crashing.

    if (oldDocument) {
        disconnect(oldDocument, &MapDocument::layerAboutToBeRemoved,
                   this, &LayerOffsetTool::abortDrag);
    }

    if (newDocument) {
        connect(newDocument, &MapDocument::layerAboutToBeRemoved,
                this, &LayerOffsetTool::abortDrag);
    }
}

void LayerOffsetTool::startDrag(const QPointF &pos)
{
    if (!mapDocument())
        return;

    const auto &layers = mapDocument()->selectedLayers();

    QVector<DraggingLayer> layersToDrag;

    LayerIterator iterator(mapDocument()->map());
    while (Layer *layer = iterator.next()) {
        if (!layer->isUnlocked())
            continue;
        if (!layers.contains(layer))
            continue;

        // Before adding a group layer, make sure none of its children are in
        // the list, to avoid applying the offset multiple times.
        if (layer->isGroupLayer())
            for (int i = layersToDrag.size() - 1; i >= 0; --i)
                if (layersToDrag.at(i).layer->isParentOrSelf(layer))
                    layersToDrag.removeAt(i);

        layersToDrag.append({ layer, layer->offset() });
    }

    if (layersToDrag.isEmpty())
        return;

    mDragging = true;
    mMouseSceneStart = pos;
    mDraggingLayers.swap(layersToDrag);
}

void LayerOffsetTool::abortDrag()
{
    QVector<DraggingLayer> draggedLayers;
    mDraggingLayers.swap(draggedLayers);
    mDragging = false;

    mMousePressed = false;  // require pressing the button again to restart the drag
    setCursor(QCursor());

    if (!mapDocument())
        return;

    mApplyingChange = true;
    for (const DraggingLayer &dragging : qAsConst(draggedLayers)) {
        dragging.layer->setOffset(dragging.oldOffset);
        emit mapDocument()->changed(LayerChangeEvent(dragging.layer, LayerChangeEvent::OffsetProperty));
    }
    mApplyingChange = false;
}

void LayerOffsetTool::finishDrag()
{
    QVector<DraggingLayer> draggedLayers;
    mDraggingLayers.swap(draggedLayers);
    mDragging = false;

    mMousePressed = false;
    setCursor(QCursor());

    if (!mapDocument() || draggedLayers.isEmpty())
        return;

    auto undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(QCoreApplication::translate("Undo Commands",
                                                      "Change Layer Offset"));

    mApplyingChange = true;
    for (const DraggingLayer &dragging : qAsConst(draggedLayers)) {
        const QPointF newOffset = dragging.layer->offset();
        dragging.layer->setOffset(dragging.oldOffset);  // restore old offset for undo command
        undoStack->push(new SetLayerOffset(mapDocument(),
                                           dragging.layer,
                                           newOffset));
    }
    mApplyingChange = false;

    undoStack->endMacro();
}
