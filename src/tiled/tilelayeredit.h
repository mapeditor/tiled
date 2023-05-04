/*
 * tilelayeredit.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editabletile.h"
#include "tilelayer.h"

#include <QObject>

namespace Tiled {

class EditableTileLayer;

class TileLayerEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Tiled::EditableTileLayer *target READ target)
    Q_PROPERTY(bool mergeable READ isMergeable WRITE setMergeable)

public:
    explicit TileLayerEdit(EditableTileLayer *tileLayer,
                           QObject *parent = nullptr);
    ~TileLayerEdit() override;

    /**
     * Sets whether this edit can be merged with a previous edit.
     *
     * Calling apply() automatically set this edit to be mergeable, so that
     * edits are merged when this object is reused.
     */
    void setMergeable(bool mergeable);
    bool isMergeable() const;

    EditableTileLayer *target() const;

public slots:
    void setTile(int x, int y, EditableTile *tile, int flags = 0);
    void apply();

private:
    EditableTileLayer *mTargetLayer;
    TileLayer mChanges;
    bool mMergeable = false;
};


inline void TileLayerEdit::setMergeable(bool mergeable)
{
    mMergeable = mergeable;
}

inline bool TileLayerEdit::isMergeable() const
{
    return mMergeable;
}

inline EditableTileLayer *TileLayerEdit::target() const
{
    return mTargetLayer;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::TileLayerEdit*)
