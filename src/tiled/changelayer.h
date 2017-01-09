/*
 * changelayer.h
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

#ifndef CHANGELAYER_H
#define CHANGELAYER_H

#include "undocommands.h"

#include <QPointF>
#include <QUndoCommand>

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * Used for changing layer visibility.
 */
class SetLayerVisible : public QUndoCommand
{
public:
    SetLayerVisible(MapDocument *mapDocument,
                    int layerIndex,
                    bool visible);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    MapDocument *mMapDocument;
    int mLayerIndex;
    bool mVisible;
};

/**
 * Used for changing layer opacity.
 */
class SetLayerOpacity : public QUndoCommand
{
public:
    SetLayerOpacity(MapDocument *mapDocument,
                    int layerIndex,
                    float opacity);

    void undo() override { setOpacity(mOldOpacity); }
    void redo() override { setOpacity(mNewOpacity); }

    int id() const override { return Cmd_ChangeLayerOpacity; }

    bool mergeWith(const QUndoCommand *other) override;

private:
    void setOpacity(float opacity);

    MapDocument *mMapDocument;
    int mLayerIndex;
    float mOldOpacity;
    float mNewOpacity;
};

/**
 * Used for changing the layer offset.
 */
class SetLayerOffset : public QUndoCommand
{
public:
    SetLayerOffset(MapDocument *mapDocument,
                   int layerIndex,
                   const QPointF &offset,
                   QUndoCommand *parent = nullptr);

    void undo() override { setOffset(mOldOffset); }
    void redo() override { setOffset(mNewOffset); }

    int id() const override { return Cmd_ChangeLayerOffset; }

private:
    void setOffset(const QPointF &offset);

    MapDocument *mMapDocument;
    int mLayerIndex;
    QPointF mOldOffset;
    QPointF mNewOffset;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGELAYER_H
