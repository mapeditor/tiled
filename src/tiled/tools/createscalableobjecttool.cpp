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

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "snaphelper.h"
#include "utils.h"

using namespace Tiled;

CreateScalableObjectTool::CreateScalableObjectTool(Id id, QObject *parent)
    : CreateObjectTool(id, parent)
{
}

bool CreateScalableObjectTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    mStartPos = pos;
    return CreateObjectTool::startNewMapObject(pos, objectGroup);
}

static qreal sign(qreal value)
{
    return value < 0 ? -1 : 1;
}

void CreateScalableObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    QPointF pixelCoords = renderer->screenToPixelCoords(pos);

    if (state() == Preview) {
        SnapHelper(renderer, modifiers).snap(pixelCoords);
        mStartPos = pixelCoords;
    }

    QRectF objectArea(mStartPos, pixelCoords);

    // Holding shift creates circle or square
    if (modifiers & Qt::ShiftModifier) {
        qreal max = qMax(qAbs(objectArea.width()), qAbs(objectArea.height()));
        objectArea.setWidth(max * sign(objectArea.width()));
        objectArea.setHeight(max * sign(objectArea.height()));
    }

    // Update the position and size of the new map object
    QPointF snapSize(objectArea.width(), objectArea.height());
    SnapHelper(renderer, modifiers).snap(snapSize);
    objectArea.setWidth(snapSize.x());
    objectArea.setHeight(snapSize.y());

    // Not using the MapObjectModel because the object is not actually part of
    // the map yet
    mNewMapObjectItem->mapObject()->setBounds(objectArea.normalized());
    mNewMapObjectItem->syncWithMapObject();
}
