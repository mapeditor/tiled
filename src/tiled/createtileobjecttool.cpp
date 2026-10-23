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
#include "stampbrush.h"
#include "tile.h"
#include "tilestamp.h"
#include "toolmanager.h"
#include "utils.h"

using namespace Tiled;

static const TileLayer *selectedTileLayer(ToolManager *toolManager)
{
    if (StampBrush *stampBrush = toolManager->findTool<StampBrush>()) {
        const TileStamp &stamp = stampBrush->stamp();
        if (!stamp.variations().isEmpty()) {
            const Map *map = stamp.variations().first().map;
            if (map && map->layerCount() > 0)
                return map->layerAt(0)->asTileLayer();
        }
    }
    return nullptr;
}

CreateTileObjectTool::CreateTileObjectTool(QObject *parent)
    : CreateObjectTool("CreateTileObjectTool", parent)
{
    QIcon icon(QLatin1String(":images/24/insert-image.png"));
    icon.addFile(QLatin1String(":images/48/insert-image.png"));
    setIcon(icon);
    setShortcut(Qt::Key_T);
    setUsesSelectedTiles(true);
    Utils::setThemeIcon(this, "insert-image");
    languageChangedImpl();
}

void CreateTileObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    MapObject *newMapObject = mNewMapObjectItem->mapObject();

    if (state() == Preview && syncSelection()) {
        newMapObject->setCell(mCell);
        newMapObject->setTileLayer(mTileLayer ? QSharedPointer<TileLayer>(mTileLayer->clone()) : nullptr);
        newMapObject->setRotation(mRotation);
        newMapObject->setSize(newMapObject->tileObjectSize(mapDocument()->map()));
        mNewMapObjectItem->update();
        mNewMapObjectItem->syncWithMapObject();
    }

    // todo: take into account rotation when positioning the preview

    const QSizeF imgSize = newMapObject->tileObjectSize(mapDocument()->map());
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

bool CreateTileObjectTool::syncSelection()
{
    const TileLayer *currentLayer = selectedTileLayer(toolManager());
    if (currentLayer && !currentLayer->isEmpty() && (currentLayer->width() > 1 || currentLayer->height() > 1)) {
        if (mSourceTileLayer != currentLayer || !mTileLayer) {
            mSourceTileLayer = currentLayer;
            mTileLayer = QSharedPointer<TileLayer>(currentLayer->clone());
            setCell(Cell());
            mRotation = 0;
            return true;
        }
        return false;
    }

    if (tile() && (mCell.tile() != tile() || mTileLayer)) {
        mSourceTileLayer = nullptr;
        mTileLayer.clear();
        setCell(Cell(tile()));
        mRotation = 0;
        return true;
    }

    return false;
}

MapObject *CreateTileObjectTool::createNewMapObject()
{
    syncSelection();

    if (!tile() && !mTileLayer)
        return nullptr;

    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Rectangle);
    newMapObject->setCell(mCell);
    newMapObject->setTileLayer(mTileLayer ? QSharedPointer<TileLayer>(mTileLayer->clone()) : nullptr);
    newMapObject->setSize(newMapObject->tileObjectSize(mapDocument()->map()));
    newMapObject->setRotation(mRotation);
    return newMapObject;
}

void CreateTileObjectTool::flipHorizontally()
{
    syncSelection();

    const Map *map = mapDocument() ? mapDocument()->map() : nullptr;
    if (mTileLayer)
        MapObject::flipTileLayer(mTileLayer.data(), FlipHorizontally, map);
    else
        mCell.setFlippedHorizontally(!mCell.flippedHorizontally());

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setCell(mCell);
        if (mTileLayer) {
            newMapObject->setTileLayer(QSharedPointer<TileLayer>(mTileLayer->clone()));
            newMapObject->setSize(newMapObject->tileObjectSize(map));
        }
        mNewMapObjectItem->syncWithMapObject();
        mNewMapObjectItem->update();
        break;
    }
    }
}

void CreateTileObjectTool::flipVertically()
{
    syncSelection();

    const Map *map = mapDocument() ? mapDocument()->map() : nullptr;
    if (mTileLayer)
        MapObject::flipTileLayer(mTileLayer.data(), FlipVertically, map);
    else
        mCell.setFlippedVertically(!mCell.flippedVertically());

    switch (state()) {
    case Idle:
        break;
    case Preview:
    case CreatingObject: {
        MapObject *newMapObject = mNewMapObjectItem->mapObject();
        newMapObject->setCell(mCell);
        if (mTileLayer) {
            newMapObject->setTileLayer(QSharedPointer<TileLayer>(mTileLayer->clone()));
            newMapObject->setSize(newMapObject->tileObjectSize(map));
        }
        mNewMapObjectItem->syncWithMapObject();
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
