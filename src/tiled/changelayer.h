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
#include <QSize>
#include <QUndoCommand>

namespace Tiled {

class Layer;
class TileLayer;

class Document;

class SetLayerName : public QUndoCommand
{
public:
    SetLayerName(Document *document,
                 Layer *layer,
                 const QString &name);

    void undo() override;
    void redo() override;

private:
    void swapName();

    Document *mDocument;
    Layer *mLayer;
    QString mName;
};

/**
 * Used for changing layer visibility.
 */
class SetLayerVisible : public QUndoCommand
{
public:
    SetLayerVisible(Document *document,
                    Layer *layer,
                    bool visible);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    Document *mDocument;
    Layer *mLayer;
    bool mVisible;
};

/**
 * Used for changing layer lock.
 */
class SetLayerLocked : public QUndoCommand
{
public:
    SetLayerLocked(Document *document,
                   Layer *layer,
                   bool locked);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    Document *mDocument;
    Layer *mLayer;
    bool mLocked;
};


/**
 * Used for changing layer opacity.
 */
class SetLayerOpacity : public QUndoCommand
{
public:
    SetLayerOpacity(Document *document,
                    Layer *layer,
                    qreal opacity);

    void undo() override { setOpacity(mOldOpacity); }
    void redo() override { setOpacity(mNewOpacity); }

    int id() const override { return Cmd_ChangeLayerOpacity; }

    bool mergeWith(const QUndoCommand *other) override;

private:
    void setOpacity(qreal opacity);

    Document *mDocument;
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
    SetLayerOffset(Document *document,
                   Layer *layer,
                   const QPointF &offset,
                   QUndoCommand *parent = nullptr);

    void undo() override { setOffset(mOldOffset); }
    void redo() override { setOffset(mNewOffset); }

    int id() const override { return Cmd_ChangeLayerOffset; }

private:
    void setOffset(const QPointF &offset);

    Document *mDocument;
    Layer *mLayer;
    QPointF mOldOffset;
    QPointF mNewOffset;
};

/**
 * Used for changing the tile layer size.
 *
 * Does not affect the contents of the tile layer, as opposed to the
 * ResizeTileLayer command.
 */
class SetTileLayerSize : public QUndoCommand
{
public:
    SetTileLayerSize(Document *document,
                     TileLayer *tileLayer,
                     QSize size,
                     QUndoCommand *parent = nullptr);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    Document *mDocument;
    TileLayer *mTileLayer;
    QSize mSize;
};

} // namespace Tiled
