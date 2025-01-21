/*
 * undocommands.h
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

class QUndoCommand;

namespace Tiled {

/**
 * These undo command IDs are used by Qt to determine whether two undo commands
 * can be merged.
 */
enum UndoCommands {
    Cmd_ChangeClassName,
    Cmd_ChangeImageLayerImageSource,
    Cmd_ChangeImageLayerRepeatX,
    Cmd_ChangeImageLayerRepeatY,
    Cmd_ChangeImageLayerTransparentColor,
    Cmd_ChangeLayerBlendMode,
    Cmd_ChangeLayerLocked,
    Cmd_ChangeLayerName,
    Cmd_ChangeLayerOffset,
    Cmd_ChangeLayerOpacity,
    Cmd_ChangeLayerParallaxFactor,
    Cmd_ChangeLayerTintColor,
    Cmd_ChangeLayerVisible,
    Cmd_ChangeMapBackgroundColor,
    Cmd_ChangeMapChunkSize,
    Cmd_ChangeMapCompressionLevel,
    Cmd_ChangeMapHexSideLength,
    Cmd_ChangeMapInfinite,
    Cmd_ChangeMapLayerDataFormat,
    Cmd_ChangeMapObject,
    Cmd_ChangeMapObjectTransform,
    Cmd_ChangeMapOrientation,
    Cmd_ChangeMapParallaxOrigin,
    Cmd_ChangeMapRenderOrder,
    Cmd_ChangeMapStaggerAxis,
    Cmd_ChangeMapStaggerIndex,
    Cmd_ChangeMapTileSize,
    Cmd_ChangeObjectGroupColor,
    Cmd_ChangeObjectGroupDrawOrder,
    Cmd_ChangeSelectedArea,
    Cmd_ChangeTileImageRect,
    Cmd_ChangeTileProbability,
    Cmd_ChangeTileWangId,
    Cmd_ChangeTilesetName,
    Cmd_ChangeTilesetTileOffset,
    Cmd_ChangeWangColorName,
    Cmd_ChangeWangSetName,
    Cmd_EraseTiles,
    Cmd_PaintTileLayer,
    Cmd_SetProperty,
};

/**
 * Interface to be implemented by undo commands that need to be clonable.
 *
 * An undo command needs to be clonable when it may be used as a child of a
 * command that may be merged with another command (which calls
 * cloneChildren()).
 */
class ClonableUndoCommand
{
public:
    virtual ~ClonableUndoCommand() = default;
    virtual QUndoCommand *clone(QUndoCommand *parent = nullptr) const = 0;
};

bool cloneChildren(const QUndoCommand *command, QUndoCommand *parent);

} // namespace Tiled
