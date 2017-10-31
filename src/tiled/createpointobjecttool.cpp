/*
 * createpointobjecttool.cpp
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

#include "createpointobjecttool.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "snaphelper.h"
#include "utils.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreatePointObjectTool::CreatePointObjectTool(QObject *parent)
    : CreateObjectTool(parent)
{
    QIcon icon(QLatin1String(":images/24x24/insert-point.png"));
    icon.addFile(QLatin1String(":images/48x48/insert-point.png"));
    setIcon(icon);
    Utils::setThemeIcon(this, "insert-point");
    languageChanged();
}

void CreatePointObjectTool::languageChanged()
{
    setName(tr("Insert Point"));
    setShortcut(QKeySequence(tr("I")));
}

void CreatePointObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos,
                                                          Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();

    QPointF pixelCoords = renderer->screenToPixelCoords(pos);
    SnapHelper(renderer, modifiers).snap(pixelCoords);

    mNewMapObjectItem->mapObject()->setPosition(pixelCoords);
    mNewMapObjectItem->syncWithMapObject();
}

void CreatePointObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        cancelNewMapObject();
}

void CreatePointObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        finishNewMapObject();
}

MapObject *CreatePointObjectTool::createNewMapObject()
{
    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Point);
    return newMapObject;
}

bool CreatePointObjectTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    if (!CreateObjectTool::startNewMapObject(pos, objectGroup))
        return false;
    return true;
}
