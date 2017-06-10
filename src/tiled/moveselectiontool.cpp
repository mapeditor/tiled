/*
 * moveselectiontool.cpp
 * Copyright 2017, Ketan Gupta <ketan19972010@gmail.com>
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

#include "moveselectiontool.h"

#include "brushitem.h"
#include "changeselectedarea.h"
#include "clipboardmanager.h"
#include "erasetiles.h"
#include "mapdocument.h"
#include "painttilelayer.h"
#include "tilesetmanager.h"

#include <QApplication>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

MoveSelectionTool::MoveSelectionTool(QObject *parent)
    : AbstractTileTool(tr("Move Selection"),
                       QIcon(QLatin1String(
                               ":images/22x22/move-selection.png")),
                       QKeySequence(tr("G")),
                       parent)
    , mDragging(false)
    , mMouseDown(false)
    , mCut(false)
{
}

MoveSelectionTool::~MoveSelectionTool()
{
}

void MoveSelectionTool::tilePositionChanged(const QPoint &pos)
{
    if (mDragging) {

        QPoint offset = pos - mLastUpdate;
        QRegion selectedArea = mapDocument()->selectedArea();
        selectedArea.translate(offset);

        mapDocument()->undoStack()->push(new ChangeSelectedArea(mapDocument(), selectedArea));

        mLastUpdate = pos;
        brushItem()->setTileRegion(mapDocument()->selectedArea());
    }
}

void MoveSelectionTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (mMouseDown && !mDragging) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mMouseScreenStart - screenPos).manhattanLength();

        if (dragDistance >= QApplication::startDragDistance() / 2) {
            mDragging = true;
            tilePositionChanged(tilePosition());
            if (!mCut) {
                mCut = true;
                cut();
            }
        }
    }

    AbstractTileTool::mouseMoved(pos, modifiers);

    refreshCursor();
}

void MoveSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && mapDocument()->selectedArea().contains(tilePosition())) {
        mMouseScreenStart = event->screenPos();
        mDragStart = tilePosition();
        mMouseDown = true;
        mLastUpdate = tilePosition();
    }
}

void MoveSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
    if (mDragging) {
        mDragging = false;
        brushItem()->setTileRegion(QRegion());
    }

    if (mCut) {
        paste();
        mCut = false;
    }

    mMouseDown = false;
    refreshCursor();
}

void MoveSelectionTool::languageChanged()
{
    setName(tr("Move Selection"));
    setShortcut(QKeySequence(tr("G")));
}

void MoveSelectionTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    if (mDragging)
        cursorShape = Qt::SizeAllCursor;

    if (cursor().shape() != cursorShape)
        setCursor(cursorShape);
}

void MoveSelectionTool::cut()
{
    Layer *currentLayer = mapDocument()->currentLayer();

    TileLayer *tileLayer = dynamic_cast<TileLayer*>(currentLayer);
    const QRegion &selectedArea = mapDocument()->selectedArea();

    ClipboardManager::instance()->copySelection(mapDocument());

    QUndoStack *stack = mapDocument()->undoStack();
    stack->beginMacro(tr("Move Selection"));

    if (tileLayer && !selectedArea.isEmpty()) {
        stack->push(new EraseTiles(mapDocument(), tileLayer, selectedArea));
    }

    stack->endMacro();
}

void MoveSelectionTool::paste()
{
    Layer *currentLayer = mapDocument()->currentLayer();

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    QScopedPointer<Map> map(clipboardManager->map());

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReferences(map->tilesets());

    mapDocument()->unifyTilesets(map.data());
    Layer *layer = map->layerAt(0);

    TileLayer *source = static_cast<TileLayer*>(layer);
    TileLayer *target = currentLayer->asTileLayer();

    QPoint offset = tilePosition() - mDragStart;

    auto undoStack = mapDocument()->undoStack();

    QRegion selectedArea = mapDocument()->selectedArea();
    undoStack->push(new ChangeSelectedArea(mapDocument(), QRegion()));

    undoStack->push(new PaintTileLayer(mapDocument(),
                                       target,
                                       source->x()+offset.x(),
                                       source->y()+offset.y(),
                                       source));

    undoStack->push(new ChangeSelectedArea(mapDocument(), selectedArea));

    if (map)
        tilesetManager->removeReferences(map->tilesets());
}