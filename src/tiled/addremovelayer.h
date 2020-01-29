/*
 * addremovelayer.h
 * Copyright 2009-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "undocommands.h"

#include <QUndoCommand>

namespace Tiled {

class GroupLayer;
class Layer;

class MapDocument;

/**
 * Abstract base class for AddLayer and RemoveLayer.
 */
class AddRemoveLayer : public QUndoCommand
{
public:
    AddRemoveLayer(MapDocument *mapDocument, int index, Layer *layer,
                   GroupLayer *parentLayer,
                   QUndoCommand *parent = nullptr);

    ~AddRemoveLayer();

protected:
    void addLayer();
    void removeLayer();

    MapDocument *mMapDocument;
    Layer *mLayer;
    GroupLayer *mParentLayer;
    int mIndex;
};

/**
 * Undo command that adds a layer to a map.
 */
class AddLayer : public AddRemoveLayer, public ClonableUndoCommand
{
public:
    /**
     * Creates an undo command that adds the \a layer to \a parentLayer at
     * \a index.
     */
    AddLayer(MapDocument *mapDocument,
             int index, Layer *layer, GroupLayer *parentLayer,
             QUndoCommand *parent = nullptr);

    void undo() override
    { removeLayer(); }

    void redo() override
    { addLayer(); }

    AddLayer *clone(QUndoCommand *parent = nullptr) const override;
};

/**
 * Undo command that removes a layer from a map.
 */
class RemoveLayer : public AddRemoveLayer
{
public:
    /**
     * Creates an undo command that removes the layer at \a index.
     */
    RemoveLayer(MapDocument *mapDocument,
                int index, GroupLayer *parentLayer,
                QUndoCommand *parent = nullptr);

    void undo() override
    { addLayer(); }

    void redo() override
    { removeLayer(); }
};

} // namespace Tiled
