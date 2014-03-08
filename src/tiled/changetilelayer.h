/*
 * changetilelayer.h
 * Copyright 2012-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#ifndef CHANGETILELAYER_H
#define CHANGETILELAYER_H

#endif // CHANGETILELAYER_H

#include <QUndoCommand>

namespace Tiled {

class TileLayer;

namespace Internal {

class MapDocument;

/**
 * Used for changing layer horizontal offset.
 */
class SetLayerHorizontalOffset : public QUndoCommand
{
public:
    SetLayerHorizontalOffset(MapDocument *mapDocument,
                             int layerIndex,
                             int offset);

    void undo() { setOffset(mOldHorizontalOffset); }
    void redo() { setOffset(mNewHorizontalOffset); }

private:
    void setOffset(int offset);

    MapDocument *mMapDocument;
    int mLayerIndex;
    int mOldHorizontalOffset;
    int mNewHorizontalOffset;
};

/**
 * Used for changing layer vertical offset.
 */
class SetLayerVerticalOffset : public QUndoCommand
{
public:
    SetLayerVerticalOffset(MapDocument *mapDocument,
                           int layerIndex,
                           int offset);

    void undo() { setOffset(mOldVerticalOffset); }
    void redo() { setOffset(mNewVerticalOffset); }

private:
    void setOffset(int offset);

    MapDocument *mMapDocument;
    int mLayerIndex;
    int mOldVerticalOffset;
    int mNewVerticalOffset;
};

} // namespace Internal
} // namespace Tiled
