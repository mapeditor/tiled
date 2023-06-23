/*
 * tilelayerwangedit.h
 * Copyright 2023, a-morphous
 * Copyright 2023, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "editablewangset.h"
#include "tilelayer.h"
#include "wangfiller.h"

#include <QObject>

#include <memory>

namespace Tiled {

class EditableTileLayer;

class TileLayerWangEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Tiled::EditableTileLayer *target READ target CONSTANT)
    Q_PROPERTY(bool mergeable READ isMergeable WRITE setMergeable)

public:
    enum Edge {
        Left = WangId::Left,
        Right = WangId::Right,
        Top = WangId::Top,
        Bottom = WangId::Bottom
    };
    Q_ENUM(Edge)

    explicit TileLayerWangEdit(EditableTileLayer *tileLayer,
                               EditableWangSet *wangSet,
                               QObject *parent = nullptr);
    ~TileLayerWangEdit() override;

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
    void setTerrain(int x, int y, int color, WangId::Index index);
    void setCorner(int x, int y, int color);
    void setEdge(int x, int y, int color, Edge direction);

    void apply();

private:
    EditableTileLayer *mTargetLayer;
    TileLayer mChanges;
    bool mMergeable = false;
    std::unique_ptr<WangFiller> mWangFiller;
};


inline void TileLayerWangEdit::setMergeable(bool mergeable)
{
    mMergeable = mergeable;
}

inline bool TileLayerWangEdit::isMergeable() const
{
    return mMergeable;
}

inline EditableTileLayer *TileLayerWangEdit::target() const
{
    return mTargetLayer;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::TileLayerWangEdit*)
