/*
 * changebezier.h
 * Copyright 2014, Martin Ziel <martin.ziel@gmail.com>
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

#ifndef CHANGEBEZIER_H
#define CHANGEBEZIER_H

#include <QPolygonF>
#include <QUndoCommand>

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;

/**
 * Changes the bezier of a MapObject.
 *
 * This class expects the bezier properties to be already changed, and takes the previous
 * bezier properties in the constructor.
 */
class ChangeBezier : public QUndoCommand
{
public:
    ChangeBezier(MapDocument *mapDocument,
                  MapObject *mapObject,
                  const QPolygonF &oldPoints,
                  const QPolygonF &oldLeftControlPoints,
                  const QPolygonF &oldRightControlPoints);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    MapObject *mMapObject;

    QPolygonF mOldPoints;
    QPolygonF mNewPoints;

    QPolygonF mOldLeftControlPoints;
    QPolygonF mNewLeftControlPoints;

    QPolygonF mOldRightControlPoints;
    QPolygonF mNewRightControlPoints;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGEBEZIER_H
