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
#include "preferences.h"
#include "utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "tile.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateTileObjectTool::CreateTileObjectTool(QObject *parent)
    : CreateObjectTool(CreateObjectTool::CreateTile, parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-image.png")));
    Utils::setThemeIcon(this, "insert-image");
    languageChanged();
}

void CreateTileObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers,
                                                         bool snapToGrid, bool snapToFineGrid)
{
    const MapRenderer *renderer = mapDocument()->renderer();

    const QSize imgSize = mNewMapObjectItem->mapObject()->cell().tile->size();
    const QPointF diff(-imgSize.width() / 2, imgSize.height() / 2);
    QPointF pixelCoords = renderer->screenToPixelCoords(pos + diff);

    if (snapToFineGrid || snapToGrid) {
        QPointF tileCoords = renderer->pixelToTileCoords(pixelCoords);

        if (snapToFineGrid) {
            int gridFine = Preferences::instance()->gridFine();
            tileCoords = (tileCoords * gridFine).toPoint();
            tileCoords /= gridFine;
        } else {
            tileCoords = tileCoords.toPoint();
        }

        pixelCoords = renderer->tileToPixelCoords(tileCoords);
    }

    mNewMapObjectItem->mapObject()->setPosition(pixelCoords);
    mNewMapObjectItem->syncWithMapObject();
    mNewMapObjectItem->setZValue(10000); // sync may change it
}

void CreateTileObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event, bool, bool)
{
    if (event->button() == Qt::RightButton)
        cancelNewMapObject();
}

void CreateTileObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event, bool, bool)
{
    if (event->button() == Qt::LeftButton)
        finishNewMapObject();
}

void CreateTileObjectTool::languageChanged()
{
    setName(tr("Insert Tile"));
    setShortcut(QKeySequence(tr("T")));
}

MapObject *CreateTileObjectTool::createNewMapObject()
{
    if(!mTile)
        return 0;

    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Rectangle);
    newMapObject->setCell(Cell(mTile));
    return newMapObject;
}
