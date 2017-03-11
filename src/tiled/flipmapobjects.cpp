/*
 * flipmapobjects.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Klimov Viktor <vitek.fomino@bk.ru>
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

#include "flipmapobjects.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

FlipMapObjects::FlipMapObjects(MapDocument *mapDocument,
                               const QList<MapObject *> &mapObjects,
                               FlipDirection flipDirection)
    : mMapDocument(mapDocument)
    , mMapObjects(mapObjects)
    , mFlipDirection(flipDirection)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Flip %n Object(s)",
                                        nullptr, mapObjects.size()));
}

void FlipMapObjects::flip()
{
    const auto &objects = mMapObjects;

    //computing objects center
    QPainterPath boundaringPath;
    for (MapObject *object : objects) {
        QTransform objectTransform;
        objectTransform.translate(object->x(), object->y());
        objectTransform.rotate(object->rotation());
        objectTransform.translate(-object->x(), -object->y());

        if(!object->cell().isEmpty()){ //computing bound rect for cell
            QRectF cellRect = QRectF(object->x(),
                                     object->y(),
                                     object->width(), -object->height()).normalized();
            boundaringPath.addRect(objectTransform.mapRect(cellRect));
        }
        else if(!object->polygon().empty()){ //computing bound rect for polygon
            const QPolygonF &objectPolygon = object->polygon();
            boundaringPath.addRect(objectTransform.mapRect(QRectF(object->position(), objectPolygon.boundingRect().size())));
        }
        else { //computing bound rect for other
            boundaringPath.addRect(objectTransform.mapRect(object->bounds()));
        }
    }
    QPointF objectsCenter = boundaringPath.boundingRect().center();

    //flip objects
    for (MapObject *object : objects)
        object->flip(mFlipDirection, objectsCenter);

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}
