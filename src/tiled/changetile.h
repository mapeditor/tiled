/*
 * changetile.h
 * Copyright 2015-2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "changevalue.h"
#include "undocommands.h"

#include <QVector>
#include <QPoint>

namespace Tiled {

class Tile;

class TilesetDocument;

class ChangeTileProbability : public ChangeValue<Tile, qreal>
{
public:
    ChangeTileProbability(TilesetDocument *tilesetDocument,
                          const QList<Tile*> &tiles,
                          qreal probability,
                          QUndoCommand *parent = nullptr);

    ChangeTileProbability(TilesetDocument *tilesetDocument,
                          const QList<Tile*> &tiles,
                          const QVector<qreal> &probabilities,
                          QUndoCommand *parent = nullptr);

    int id() const override { return Cmd_ChangeTileProbability; }

protected:
    qreal getValue(const Tile *tile) const override;
    void setValue(Tile *tile, const qreal &probability) const override;
};

class ChangeTileImageRect : public ChangeValue<Tile, QRect>
{
public:
    ChangeTileImageRect(TilesetDocument *tilesetDocument,
                        const QList<Tile*> &tiles,
                        const QVector<QRect> &rects,
                        QUndoCommand *parent = nullptr);

    int id() const override { return Cmd_ChangeTileImageRect; }

protected:
    QRect getValue(const Tile *tile) const override;
    void setValue(Tile *tile, const QRect &rect) const override;
};

class ChangeTileOrigin : public ChangeValue<Tile, QPoint>
{
public:
    ChangeTileOrigin(TilesetDocument *tilesetDocument,
                     const QList<Tile*> &tiles,
                     const QPoint &origins,
                     QUndoCommand *parent = nullptr);

    ChangeTileOrigin(TilesetDocument *tilesetDocument,
                     const QList<Tile*> &tiles,
                     const QVector<QPoint> &origins,
                     QUndoCommand *parent = nullptr);

    int id() const override { return Cmd_ChangeTileOrigin; }

protected:
    QPoint getValue(const Tile *tile) const override;
    void setValue(Tile *tile, const QPoint &origin) const override;
};

} // namespace Tiled
