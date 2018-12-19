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

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"
#include "objectgroup.h"

#include <QCoreApplication>

using namespace Tiled;

ChangePolygon::ChangePolygon(MapDocument *mapDocument,
                             MapObject *mapObject,
                             const QPolygonF &oldPolygon)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mOldPolygon(oldPolygon)
    , mNewPolygon(mapObject->polygon())
    , mOldChangeState(mapObject->propertyChanged(MapObject::ShapeProperty))
{
    setText(QCoreApplication::translate("Undo Commands", "Change Polygon"));
}

ChangePolygon::ChangePolygon(MapDocument *mapDocument,
                             MapObject *mapObject,
                             const QPolygonF &newPolygon,
                             const QPolygonF &oldPolygon)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mOldPolygon(oldPolygon)
    , mNewPolygon(newPolygon)
    , mOldChangeState(mapObject->propertyChanged(MapObject::ShapeProperty))
{
    setText(QCoreApplication::translate("Undo Commands", "Change Polygon"));
}

void ChangePolygon::undo()
{
    mMapDocument->mapObjectModel()->setObjectPolygon(mMapObject, mOldPolygon);
    mMapObject->setPropertyChanged(MapObject::ShapeProperty, mOldChangeState);
}

void ChangePolygon::redo()
{
    mMapDocument->mapObjectModel()->setObjectPolygon(mMapObject, mNewPolygon);
    mMapObject->setPropertyChanged(MapObject::ShapeProperty);
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
    , mSecondPolyline(nullptr)
    , mEdgeIndex(index)
    , mObjectIndex(-1)
    , mOldChangeState(mapObject->propertyChanged(MapObject::ShapeProperty))
    , mOwnsSecondPolyline(false)
{
    setText(QCoreApplication::translate("Undo Commands", "Split Polyline"));
}

SplitPolyline::~SplitPolyline()
{
    if (mOwnsSecondPolyline)
        delete mSecondPolyline;
}

void SplitPolyline::undo()
{
    mMapDocument->mapObjectModel()->removeObject(mFirstPolyline->objectGroup(),
                                                 mSecondPolyline);
    mOwnsSecondPolyline = true;

    QPolygonF polygon = mFirstPolyline->polygon() + mSecondPolyline->polygon();

    mMapDocument->mapObjectModel()->setObjectPolygon(mFirstPolyline, polygon);
    mFirstPolyline->setPropertyChanged(MapObject::ShapeProperty, mOldChangeState);
}

void SplitPolyline::redo()
{
    QPolygonF firstPolygon = mFirstPolyline->polygon();
    firstPolygon.erase(firstPolygon.begin() + mEdgeIndex + 1, firstPolygon.end());

    if (!mSecondPolyline) {
        mObjectIndex = mFirstPolyline->objectGroup()->objects().indexOf(mFirstPolyline) + 1;

        QPolygonF secondPolygon = mFirstPolyline->polygon();
        secondPolygon.erase(secondPolygon.begin(), secondPolygon.begin() + mEdgeIndex + 1);

        mSecondPolyline = mFirstPolyline->clone();
        mSecondPolyline->resetId();
        mSecondPolyline->setPolygon(secondPolygon);
        mSecondPolyline->setPropertyChanged(MapObject::ShapeProperty);
    }

    mMapDocument->mapObjectModel()->insertObject(mFirstPolyline->objectGroup(),
                                                 mObjectIndex,
                                                 mSecondPolyline);
    mOwnsSecondPolyline = false;

    mMapDocument->mapObjectModel()->setObjectPolygon(mFirstPolyline, firstPolygon);
    mFirstPolyline->setPropertyChanged(MapObject::ShapeProperty);

    // If the first polyline is selected, select the second as well
    QList<MapObject*> selection = mMapDocument->selectedObjects();
    if (selection.contains(mFirstPolyline)) {
        selection.append(mSecondPolyline);
        mMapDocument->setSelectedObjects(selection);
    }
}
