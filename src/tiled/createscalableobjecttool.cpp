/*
 * createscalableobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel.com>
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

#include "createscalableobjecttool.h"
#include "preferences.h"
#include "utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateScalableObjectTool::CreateScalableObjectTool(QObject* parent)
    : CreateObjectTool(CreateObjectTool::CreateGeometry, parent)
{
}

void CreateScalableObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers,
                                                             const bool snapToGrid, const bool snapToFineGrid)
{
    const MapRenderer *renderer = mapDocument()->renderer();

    const QPointF pixelCoords = renderer->screenToPixelCoords(pos);

    // Update the size of the new map object
    const QPointF objectPos = mNewMapObjectItem->mapObject()->position();
    QPointF newSize(qMax(qreal(0), pixelCoords.x() - objectPos.x()),
                    qMax(qreal(0), pixelCoords.y() - objectPos.y()));
    QPointF newTileSize = renderer->pixelToTileCoords(newSize);

    if (snapToFineGrid) {
        int gridFine = Preferences::instance()->gridFine();
        newTileSize = (newTileSize * gridFine).toPoint();
        newTileSize /= gridFine;
    } else if (snapToGrid)
        newTileSize = newTileSize.toPoint();

    // Holding shift creates circle or square
    if (modifiers & Qt::ShiftModifier) {
        qreal max = qMax(newTileSize.x(), newTileSize.y());
        newTileSize.setX(max);
        newTileSize.setY(max);
    }

    newSize = renderer->tileToPixelCoords(newTileSize);

    mNewMapObjectItem->resizeObject(QSizeF(newSize.x(), newSize.y()));
}

void CreateScalableObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event,
                                                               const bool, const bool)
{
    if (event->button() == Qt::RightButton)
        cancelNewMapObject();
}

void CreateScalableObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event,
                                                                const bool, const bool)
{
    if (event->button() == Qt::LeftButton) {
        finishNewMapObject();
    }
}

void CreateScalableObjectTool::languageChanged(){
    setName(tr("Insert Rectangle"));
    setShortcut(QKeySequence(tr("R")));
}
