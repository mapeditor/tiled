/*
 * changemapproperty.h
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
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

#include "map.h"

#include <QColor>
#include <QUndoCommand>

namespace Tiled {

class MapDocument;

class ChangeMapProperty : public QUndoCommand
{
public:
    enum Property {
        TileWidth,
        TileHeight,
        Infinite,
        HexSideLength,
        StaggerAxis,
        StaggerIndex,
        Orientation,
        RenderOrder,
        BackgroundColor,
        LayerDataFormat,
        CompressionLevel,
        ChunkSize
    };

    /**
     * Constructs a command that changes the value of the given property.
     *
     * Can only be used for the HexSideLength property.
     *
     * @param mapDocument       the map document of the map
     * @param backgroundColor   the new color to apply for the background
     */
    ChangeMapProperty(MapDocument *mapDocument, Property property, int value);

    /**
     * Constructs a command that changes the map background color.
     *
     * @param mapDocument       the map document of the map
     * @param backgroundColor   the new color to apply for the background
     */
    ChangeMapProperty(MapDocument *mapDocument, const QColor &backgroundColor);
    
    /**
     * Constructs a command that changes the chunk size.
     *
     * @param mapDocument       the map document of the map
     * @param chunkSize         the new chunk size to use for tile layers
     */
    ChangeMapProperty(MapDocument *mapDocument, QSize chunkSize);

    /**
     * Constructs a command that changes the map stagger axis.
     *
     * @param mapDocument       the map document of the map
     * @param orientation       the new map stagger axis
     */
    ChangeMapProperty(MapDocument *mapDocument, Map::StaggerAxis staggerAxis);

    /**
     * Constructs a command that changes the map stagger index.
     *
     * @param mapDocument       the map document of the map
     * @param orientation       the new map stagger index
     */
    ChangeMapProperty(MapDocument *mapDocument, Map::StaggerIndex staggerIndex);

    /**
     * Constructs a command that changes the map orientation.
     *
     * @param mapDocument       the map document of the map
     * @param orientation       the new map orientation
     */
    ChangeMapProperty(MapDocument *mapDocument, Map::Orientation orientation);

    /**
     * Constructs a command that changes the render order.
     *
     * @param mapDocument       the map document of the map
     * @param renderOrder       the new map render order
     */
    ChangeMapProperty(MapDocument *mapDocument, Map::RenderOrder renderOrder);

    /**
     * Constructs a command that changes the layer data format.
     *
     * @param mapDocument       the map document of the map
     * @param layerDataFormat   the new layer data format
     */
    ChangeMapProperty(MapDocument *mapDocument, Map::LayerDataFormat layerDataFormat);

    void undo() override;
    void redo() override;

private:
    void swap();

    MapDocument *mMapDocument;
    Property mProperty;
    QColor mBackgroundColor;
    QSize mChunkSize;
    union {
        int mIntValue;
        Map::StaggerAxis mStaggerAxis;
        Map::StaggerIndex mStaggerIndex;
        Map::Orientation mOrientation;
        Map::RenderOrder mRenderOrder;
        Map::LayerDataFormat mLayerDataFormat;
    };
};

} // namespace Tiled
