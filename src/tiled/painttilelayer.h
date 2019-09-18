/*
 * painttilelayer.h
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

#pragma once

#include "undocommands.h"

#include <QRegion>
#include <QUndoCommand>

#include <memory>
#include <unordered_map>

namespace Tiled {

class TileLayer;

class MapDocument;

/**
 * A command that paints one tile layer on top of another tile layer.
 *
 * Can merge with additional commands, even when they paint on different
 * tile layers.
 */
class PaintTileLayer : public QUndoCommand
{
public:
    /**
     * Constructor.
     *
     * @param mapDocument the map document that's being edited
     * @param target      the target layer to paint on
     * @param x           the x position of the paint location
     * @param y           the y position of the paint location
     * @param source      the source layer to paint on the target layer
     */
    PaintTileLayer(MapDocument *mapDocument,
                   TileLayer *target,
                   int x, int y,
                   const TileLayer *source,
                   QUndoCommand *parent = nullptr);

    /**
     * Constructor that takes an explicit paint region.
     *
     * @param mapDocument the map document that's being edited
     * @param target      the target layer to paint on
     * @param x           the x position of the paint location
     * @param y           the y position of the paint location
     * @param source      the source layer to paint on the target layer
     * @param paintRegion the region to paint, in map coordinates
     */
    PaintTileLayer(MapDocument *mapDocument,
                   TileLayer *target,
                   int x, int y,
                   const TileLayer *source,
                   const QRegion &paintRegion,
                   QUndoCommand *parent = nullptr);

    ~PaintTileLayer() override;

    /**
     * Sets whether this undo command can be merged with an existing command.
     */
    void setMergeable(bool mergeable);

    void undo() override;
    void redo() override;

    int id() const override { return Cmd_PaintTileLayer; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    struct LayerData
    {
        void mergeWith(const LayerData &o);

        std::unique_ptr<TileLayer> mSource;
        std::unique_ptr<TileLayer> mErased;
        int mX, mY;
        QRegion mPaintedRegion;
    };

    MapDocument *mMapDocument;
    std::unordered_map<TileLayer*, LayerData> mLayerData;
    bool mMergeable;
};

inline void PaintTileLayer::setMergeable(bool mergeable)
{
    mMergeable = mergeable;
}

} // namespace Tiled
