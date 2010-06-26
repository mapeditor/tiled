/*
 * addremovelayer.h
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef ADDREMOVELAYER_H
#define ADDREMOVELAYER_H

#include <QCoreApplication>
#include <QUndoCommand>

namespace Tiled {

class Layer;

namespace Internal {

class MapDocument;

/**
 * Abstract base class for AddLayer and RemoveLayer.
 */
class AddRemoveLayer : public QUndoCommand
{
public:
    AddRemoveLayer(MapDocument *mapDocument, int index, Layer *layer);

    ~AddRemoveLayer();

protected:
    void addLayer();
    void removeLayer();

private:
    MapDocument *mMapDocument;
    Layer *mLayer;
    int mIndex;
};

/**
 * Undo command that adds a layer to a map.
 */
class AddLayer : public AddRemoveLayer
{
public:
    /**
     * Creates an undo command that adds the \a layer at \a index.
     */
    AddLayer(MapDocument *mapDocument, int index, Layer *layer)
        : AddRemoveLayer(mapDocument, index, layer)
    {
        setText(QCoreApplication::translate("Undo Commands", "Add Layer"));
    }

    void undo()
    { removeLayer(); }

    void redo()
    { addLayer(); }
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
    RemoveLayer(MapDocument *mapDocument, int index)
        : AddRemoveLayer(mapDocument, index, 0)
    {
        setText(QCoreApplication::translate("Undo Commands", "Remove Layer"));
    }

    void undo()
    { addLayer(); }

    void redo()
    { removeLayer(); }
};

} // namespace Internal
} // namespace Tiled

#endif // ADDREMOVELAYER_H
