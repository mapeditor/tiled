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

#include "changeevents.h"
#include "document.h"
#include "mapobject.h"

#include <QCoreApplication>

using namespace Tiled;

FlipMapObjects::FlipMapObjects(Document *document,
                               const QList<MapObject *> &mapObjects,
                               FlipDirection flipDirection)
    : mDocument(document)
    , mMapObjects(mapObjects)
    , mFlipDirection(flipDirection)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Flip %n Object(s)",
                                        nullptr, mapObjects.size()));

    //computing objects center
    QRectF boundaryObjectsRect;
    for (MapObject *object : mMapObjects) {
        QTransform objectTransform;
        objectTransform.translate(object->x(), object->y());
        objectTransform.rotate(object->rotation());
        objectTransform.translate(-object->x(), -object->y());

        if (!object->cell().isEmpty()) { //computing bound rect for cell
            QRectF cellRect = QRectF(object->x(),
                                     object->y(),
                                     object->width(), -object->height()).normalized();
            boundaryObjectsRect = boundaryObjectsRect.united(objectTransform.mapRect(cellRect));
        } else if (!object->polygon().empty()) { //computing bound rect for polygon
            const QPolygonF &objectPolygon = object->polygon();
            QTransform polygonToMapTransform;
            polygonToMapTransform.translate(object->x(),
                                            object->y());
            polygonToMapTransform.rotate(object->rotation());
            boundaryObjectsRect = boundaryObjectsRect.united(polygonToMapTransform.mapRect(QRectF(objectPolygon.boundingRect())));
        } else { //computing bound rect for other
            boundaryObjectsRect = boundaryObjectsRect.united(objectTransform.mapRect(object->bounds()));
        }

        mOldCellStates.append(object->propertyChanged(MapObject::CellProperty));
        mNewCellStates.append(true);

        mOldRotationStates.append(object->propertyChanged(MapObject::RotationProperty));
        mNewRotationStates.append(true);
    }
    mObjectsCenter = boundaryObjectsRect.center();
}

void FlipMapObjects::flip()
{
    //flip objects
    for (int i = 0; i < mMapObjects.size(); ++i) {
        mMapObjects[i]->flip(mFlipDirection, mObjectsCenter);

        mMapObjects[i]->setPropertyChanged(MapObject::CellProperty, mNewCellStates[i]);
        mMapObjects[i]->setPropertyChanged(MapObject::RotationProperty, mNewRotationStates[i]);
    }

    mOldRotationStates.swap(mNewRotationStates);

    constexpr MapObject::ChangedProperties changedProperties {
        MapObject::CellProperty,
        MapObject::PositionProperty,
        MapObject::RotationProperty,
        MapObject::ShapeProperty,
    };
    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, changedProperties));
}
