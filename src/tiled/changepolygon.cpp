/*
 * changepolygon.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changepolygon.h"

#include "changeevents.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"
#include "objectgroup.h"

#include <QCoreApplication>

using namespace Tiled;

ChangePolygon::ChangePolygon(Document *document,
                             MapObject *mapObject,
                             const QPolygonF &oldPolygon)
    : ChangePolygon(document,
                    mapObject,
                    mapObject->polygon(),
                    oldPolygon)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Polygon"));
}

ChangePolygon::ChangePolygon(Document *document,
                             MapObject *mapObject,
                             const QPolygonF &newPolygon,
                             const QPolygonF &oldPolygon)
    : mDocument(document)
    , mMapObject(mapObject)
    , mOldPolygon(oldPolygon)
    , mNewPolygon(newPolygon)
    , mOldChangeState(mapObject->propertyChanged(MapObject::ShapeProperty))
{
    setText(QCoreApplication::translate("Undo Commands", "Change Polygon"));
}

void ChangePolygon::undo()
{
    mMapObject->setPolygon(mOldPolygon);
    mMapObject->setPropertyChanged(MapObject::ShapeProperty, mOldChangeState);

    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, MapObject::ShapeProperty));
}

void ChangePolygon::redo()
{
    mMapObject->setPolygon(mNewPolygon);
    mMapObject->setPropertyChanged(MapObject::ShapeProperty);

    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, MapObject::ShapeProperty));
}


TogglePolygonPolyline::TogglePolygonPolyline(MapObject *mapObject)
    : mMapObject(mapObject)
{
    setText(QCoreApplication::translate("Undo Commands", "Toggle Polygon/Polyline"));
}

void TogglePolygonPolyline::toggle()
{
    mMapObject->setShape((mMapObject->shape() == MapObject::Polygon) ? MapObject::Polyline : MapObject::Polygon);
}


SplitPolyline::SplitPolyline(MapDocument *mapDocument,
                             MapObject *mapObject,
                             int index)
    : mMapDocument(mapDocument)
    , mFirstPolyline(mapObject)
    , mEdgeIndex(index)
    , mOldChangeState(mapObject->propertyChanged(MapObject::ShapeProperty))
{
    setText(QCoreApplication::translate("Undo Commands", "Split Polyline"));
}

SplitPolyline::~SplitPolyline()
{
}

void SplitPolyline::undo()
{
    Q_ASSERT(mAddSecondPolyline);

    mAddSecondPolyline->undo();

    QPolygonF polygon = mFirstPolyline->polygon() + mSecondPolyline->polygon();

    mFirstPolyline->setPolygon(polygon);
    mFirstPolyline->setPropertyChanged(MapObject::ShapeProperty, mOldChangeState);

    emit mMapDocument->changed(MapObjectsChangeEvent(mFirstPolyline, MapObject::ShapeProperty));
}

void SplitPolyline::redo()
{
    QPolygonF firstPolygon = mFirstPolyline->polygon();
    firstPolygon.erase(firstPolygon.begin() + mEdgeIndex + 1, firstPolygon.end());

    if (!mAddSecondPolyline) {
        QPolygonF secondPolygon = mFirstPolyline->polygon();
        secondPolygon.erase(secondPolygon.begin(), secondPolygon.begin() + mEdgeIndex + 1);

        mSecondPolyline = mFirstPolyline->clone();
        mSecondPolyline->resetId();
        mSecondPolyline->setPolygon(secondPolygon);
        mSecondPolyline->setPropertyChanged(MapObject::ShapeProperty);

        AddRemoveMapObjects::Entry entry;
        entry.mapObject = mSecondPolyline;
        entry.objectGroup = mFirstPolyline->objectGroup();
        entry.index = mFirstPolyline->objectGroup()->objects().indexOf(mFirstPolyline) + 1;

        mAddSecondPolyline.reset(new AddMapObjects(mMapDocument, { entry }));
    }

    mAddSecondPolyline->redo();

    mFirstPolyline->setPolygon(firstPolygon);
    mFirstPolyline->setPropertyChanged(MapObject::ShapeProperty);

    emit mMapDocument->changed(MapObjectsChangeEvent(mFirstPolyline, MapObject::ShapeProperty));

    // If the first polyline is selected, select the second as well
    QList<MapObject*> selection = mMapDocument->selectedObjects();
    if (selection.contains(mFirstPolyline)) {
        selection.append(mSecondPolyline);
        mMapDocument->setSelectedObjects(selection);
    }
}
