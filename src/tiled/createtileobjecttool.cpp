/*
 * createtileobjecttool.cpp
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

#include "createtileobjecttool.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "snaphelper.h"
#include "tile.h"
#include "utils.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateTileObjectTool::CreateTileObjectTool(QObject *parent)
    : CreateObjectTool(CreateObjectTool::CreateTile, parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-image.png")));
    Utils::setThemeIcon(this, "insert-image");
    languageChanged();
}

void CreateTileObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();

    const QSize imgSize = mNewMapObjectItem->mapObject()->cell().tile()->size();
    const QPointF diff(-imgSize.width() / 2, imgSize.height() / 2);
    QPointF pixelCoords = renderer->screenToPixelCoords(pos + diff);

    SnapHelper(renderer, modifiers).snap(pixelCoords);

    mNewMapObjectItem->mapObject()->setPosition(pixelCoords);
    mNewMapObjectItem->syncWithMapObject();
    mNewMapObjectItem->setZValue(10000); // sync may change it
    mNewMapObjectItem->setOpacity(0.75);
}

void CreateTileObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        cancelNewMapObject();
}

void CreateTileObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        finishNewMapObject();
}

void CreateTileObjectTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    CreateObjectTool::startNewMapObject(pos, objectGroup);
    if (mNewMapObjectItem)
        mNewMapObjectItem->setOpacity(0.75);
}

void CreateTileObjectTool::languageChanged()
{
    setName(tr("Insert Tile"));
    setShortcut(QKeySequence(tr("T")));
}

MapObject *CreateTileObjectTool::createNewMapObject()
{
    if (!mTile)
        return nullptr;

    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Rectangle);
    newMapObject->setCell(Cell(mTile));
    newMapObject->setSize(mTile->size());
    return newMapObject;
}
