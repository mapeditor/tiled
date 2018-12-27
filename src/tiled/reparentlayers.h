/*
 * reparentlayers.h
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include <QUndoCommand>
#include <QVector>

namespace Tiled {

class GroupLayer;
class Layer;

class MapDocument;

/**
 * Undo command that changes the parent or index of a given set of layers.
 */
class ReparentLayers : public QUndoCommand
{
public:
    ReparentLayers(MapDocument *mapDocument,
                   const QList<Layer *> &layers,
                   GroupLayer *layerParent,
                   int index,
                   QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    MapDocument * const mMapDocument;
    QList<Layer *> mLayers;
    GroupLayer * const mLayerParent;
    int const mIndex;

    struct UndoInfo {
        GroupLayer *parent;
        int oldIndex;
    };
    QVector<UndoInfo> mUndoInfo;
};

} // namespace Tiled
