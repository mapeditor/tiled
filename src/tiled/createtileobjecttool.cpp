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
#include "objectgroup.h"
#include "snaphelper.h"
#include "tile.h"
#include "utils.h"

using namespace Tiled;

CreateTileObjectTool::CreateTileObjectTool(QObject *parent)
    : CreateObjectTool("CreateTileObjectTool", parent)
{
    QIcon icon(QLatin1String(":images/24/insert-image.png"));
    icon.addFile(QLatin1String(":images/48/insert-image.png"));
    setIcon(icon);
    setShortcut(Qt::Key_T);
    Utils::setThemeIcon(this, "insert-image");
    languageChangedImpl();
}

void CreateTileObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    const QSize imgSize = mNewMapObjectItem->mapObject()->cell().tile()->size();
    const QPointF halfSize(imgSize.width() / 2, imgSize.height() / 2);
    const QRectF screenBounds { pos - halfSize, imgSize };

    // These screenBounds assume TopLeft alignment, but the map's object alignment might be different.
    MapObject *newMapObject = mNewMapObjectItem->mapObject();
    const QPointF offset = alignmentOffset(screenBounds, newMapObject->alignment(mapDocument()->map()));

    const MapRenderer *renderer = mapDocument()->renderer();
    QPointF pixelCoords = renderer->screenToPixelCoords(screenBounds.topLeft() + offset);

    SnapHelper(renderer, modifiers).snap(pixelCoords);

    newMapObject->setPosition(pixelCoords);
    mNewMapObjectItem->syncWithMapObject();
}

void CreateTileObjectTool::languageChanged()
{
    CreateObjectTool::languageChanged();
    languageChangedImpl();
}

void CreateTileObjectTool::languageChangedImpl()
{
    setName(tr("Insert Tile"));
}

MapObject *CreateTileObjectTool::createNewMapObject()
{
    if (!tile())
        return nullptr;

    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Rectangle);
    newMapObject->setCell(Cell(tile()));
    newMapObject->setSize(tile()->size());
    return newMapObject;
}
