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
    MapObject *newMapObject = mNewMapObjectItem->mapObject();

    if (state() == Preview && tile() && mCell.tile() != tile()) {
        setCell(Cell(tile()));
        mRotation = 0;

        newMapObject->setCell(mCell);
        newMapObject->setRotation(mRotation);
        newMapObject->setSize(tile()->size());
        mNewMapObjectItem->update();
        mNewMapObjectItem->syncWithMapObject();
    }

    // todo: take into account rotation when positioning the preview

    const QSize imgSize = newMapObject->cell().tile()->size();
    const QPointF halfSize(imgSize.width() / 2, imgSize.height() / 2);
    const QRectF screenBounds { pos - halfSize, imgSize };

    // These screenBounds assume TopLeft alignment, but the map's object alignment might be different.
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

    if (mCell.tile() != tile()) {
        setCell(Cell(tile()));
        mRotation = 0;
    }

    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Rectangle);
    newMapObject->setCell(mCell);
    newMapObject->setSize(tile()->size());
    newMapObject->setRotation(mRotation);
    return newMapObject;
}

void CreateTileObjectTool::flipHorizontally()
{
    mCell.setFlippedHorizontally(!mCell.flippedHorizontally());

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setCell(mCell);
        mNewMapObjectItem->update();
        break;
    }
    }
}

void CreateTileObjectTool::flipVertically()
{
    mCell.setFlippedVertically(!mCell.flippedVertically());

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setCell(mCell);
        mNewMapObjectItem->update();
        break;
    }
    }
}

void CreateTileObjectTool::rotateLeft()
{
    mRotation -= 90;
    if (mRotation < -180)
        mRotation += 360;

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setRotation(mRotation);
        mNewMapObjectItem->syncWithMapObject();
        break;
    }
    }
}

void CreateTileObjectTool::rotateRight()
{
    mRotation += 90;
    if (mRotation > 180)
        mRotation -= 360;

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setRotation(mRotation);
        mNewMapObjectItem->syncWithMapObject();
        break;
    }
    }
}

void CreateTileObjectTool::setCell(const Cell &cell)
{
    mCell = cell;
    mTileset = cell.tileset() ? cell.tileset()->sharedFromThis()
                              : SharedTileset();
}

#include "moc_createtileobjecttool.cpp"
