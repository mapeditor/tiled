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

#pragma once

#include "undocommands.h"

#include <QPointF>
#include <QUndoCommand>

namespace Tiled {

class Layer;

namespace Internal {

class MapDocument;

/**
 * Used for changing layer visibility.
 */
class SetLayerVisible : public QUndoCommand
{
public:
    SetLayerVisible(MapDocument *mapDocument,
                    Layer *layer,
                    bool visible);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    MapDocument *mMapDocument;
    Layer *mLayer;
    bool mVisible;
};

/**
 * Used for changing layer lock.
 */
class SetLayerLocked : public QUndoCommand
{
public:
    SetLayerLocked(MapDocument *mapDocument,
                   Layer *layer,
                   bool locked);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    MapDocument *mMapDocument;
    Layer *mLayer;
    bool mLocked;
};


/**
 * Used for changing layer opacity.
 */
class SetLayerOpacity : public QUndoCommand
{
public:
    SetLayerOpacity(MapDocument *mapDocument,
                    Layer *layer,
                    qreal opacity);

    void undo() override { setOpacity(mOldOpacity); }
    void redo() override { setOpacity(mNewOpacity); }

    int id() const override { return Cmd_ChangeLayerOpacity; }

    bool mergeWith(const QUndoCommand *other) override;

private:
    void setOpacity(qreal opacity);

    MapDocument *mMapDocument;
    Layer *mLayer;
    qreal mOldOpacity;
    qreal mNewOpacity;
};

/**
 * Used for changing the layer offset.
 */
class SetLayerOffset : public QUndoCommand
{
public:
    SetLayerOffset(MapDocument *mapDocument,
                   Layer *layer,
                   const QPointF &offset,
                   QUndoCommand *parent = nullptr);

    void undo() override { setOffset(mOldOffset); }
    void redo() override { setOffset(mNewOffset); }

    int id() const override { return Cmd_ChangeLayerOffset; }

private:
    void setOffset(const QPointF &offset);

    MapDocument *mMapDocument;
    Layer *mLayer;
    QPointF mOldOffset;
    QPointF mNewOffset;
};

} // namespace Internal
} // namespace Tiled
