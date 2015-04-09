/*
 * raiselowerhelper.h
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef RAISELOWERHELPER_H
#define RAISELOWERHELPER_H

#include "mapscene.h"
#include "rangeset.h"

#include <QList>

class QUndoCommand;

namespace Tiled {

class ObjectGroup;

namespace Internal {

class MapDocument;
class MapObjectItem;
class MapScene;

/**
 * Implements operations to raise or lower the set of selected objects.
 *
 * The operations don't do anything when there are multiple object groups
 * active in the selection, or when the object group does not use index drawing
 * order.
 */
class RaiseLowerHelper
{
public:
    RaiseLowerHelper(MapScene *mapScene)
        : mMapDocument(mapScene->mapDocument())
        , mMapScene(mapScene)
        , mObjectGroup(0)
    {}

    void raise();
    void lower();
    void raiseToTop();
    void lowerToBottom();

    static ObjectGroup *sameObjectGroup(const QSet<MapObjectItem*> &items);

private:
    bool initContext();
    void push(const QList<QUndoCommand *> &commands, const QString &text);

    MapDocument *mMapDocument;
    MapScene *mMapScene;

    // Context
    ObjectGroup *mObjectGroup;
    QList<MapObjectItem*> mRelatedObjects;
    RangeSet<int> mSelectionRanges;
};

} // namespace Internal
} // namespace Tiled

#endif // RAISELOWERHELPER_H
