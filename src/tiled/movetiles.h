/*
 * movetiles.h
 * Copyright 2025, Tiled contributors
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

namespace Tiled {

class TileLayer;
class MapDocument;

class MoveTiles : public QUndoCommand
{
public:
    MoveTiles(MapDocument *mapDocument,
              TileLayer *layer,
              const QRegion &sourceRegion,
              QPoint offset,
              bool duplicate,
              QUndoCommand *parent = nullptr);

    ~MoveTiles() override;

    void undo() override;
    void redo() override;

    int id() const override { return Cmd_MoveTiles; }

private:
    MapDocument *mMapDocument;
    TileLayer *mLayer;

    std::unique_ptr<TileLayer> mOriginalSourceTiles;
    QRegion mSourceRegion;

    std::unique_ptr<TileLayer> mOriginalDestTiles;
    QRegion mDestRegion;

    std::unique_ptr<TileLayer> mMovedTiles;

    QPoint mOffset;
    bool mDuplicate;
};

} // namespace Tiled
