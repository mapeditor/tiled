/*
 * movepoints.h
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

#ifndef MOVEPOINTS_H
#define MOVEPOINTS_H

#include <QPointF>
#include <QUndoCommand>
#include <QVector>

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;

/**
 * Moves one or more points of the polygon of a MapObject.
 *
 * This class expects the points to be already moved, and takes the old
 * positions of the moved points in the constructor.
 */
class MovePoints : public QUndoCommand
{
public:
    MovePoints(MapDocument *mapDocument,
               MapObject *mapObject,
               const QVector<QPointF> &oldPositions,
               const QVector<int> &pointIndexes);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    MapObject *mMapObject;

    // Using QVector since a QList would cause each point to be allocated on
    // the heap.
    QVector<QPointF> mOldPositions;
    QVector<QPointF> mNewPositions;
    QVector<int> mPointIndexes;
};

} // namespace Internal
} // namespace Tiled

#endif // MOVEPOINTS_H
