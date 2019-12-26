/*
 * erasetiles.h
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

#include <QHash>
#include <QRegion>
#include <QUndoCommand>

namespace Tiled {

class Tile;
class TileLayer;

class MapDocument;

class EraseTiles : public QUndoCommand
{
public:
    EraseTiles(MapDocument *mapDocument,
               TileLayer *tileLayer,
               const QRegion &region);
    ~EraseTiles() override;

    /**
     * Sets whether this undo command can be merged with an existing command.
     */
    void setMergeable(bool mergeable)
    { mMergeable = mergeable; }

    void undo() override;
    void redo() override;

    int id() const override { return Cmd_EraseTiles; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    struct LayerData
    {
        void mergeWith(const LayerData &o);

        TileLayer *mErasedCells = nullptr;
        QRegion mRegion;
    };

    MapDocument *mMapDocument;
    QHash<TileLayer*, LayerData> mLayerData;
    bool mMergeable;
};

} // namespace Tiled
