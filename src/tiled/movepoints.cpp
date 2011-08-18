/*
 * movepoints.cpp
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

#include "movepoints.h"

#include "mapdocument.h"
#include "mapobject.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

MovePoints::MovePoints(MapDocument *mapDocument, MapObject *mapObject,
                       const QVector<QPointF> &oldPositions,
                       const QVector<int> &pointIndexes)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mOldPositions(oldPositions)
    , mPointIndexes(pointIndexes)
{
    Q_ASSERT(oldPositions.size() == pointIndexes.size());

    const int pointCount = pointIndexes.size();
    const QPolygonF &polygon = mapObject->polygon();

    for (int i = 0; i < pointCount; ++i)
        mNewPositions.append(polygon.at(pointIndexes.at(i)));

    setText(QCoreApplication::translate("Undo Commands",
                                        "Move %n Point(s)", "",
                                        QCoreApplication::CodecForTr,
                                        pointCount));
}

void MovePoints::undo()
{
    QPolygonF polygon = mMapObject->polygon();
    for (int i = mPointIndexes.size() - 1; i >= 0; --i)
        polygon[mPointIndexes.at(i)] = mOldPositions.at(i);
    mMapObject->setPolygon(polygon);
    mMapDocument->emitObjectChanged(mMapObject);
}

void MovePoints::redo()
{
    QPolygonF polygon = mMapObject->polygon();
    for (int i = mPointIndexes.size() - 1; i >= 0; --i)
        polygon[mPointIndexes.at(i)] = mNewPositions.at(i);
    mMapObject->setPolygon(polygon);
    mMapDocument->emitObjectChanged(mMapObject);
}
