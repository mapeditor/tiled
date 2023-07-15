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
#include "map.h"

#include <QObject>

#include <memory>

namespace Tiled {

class EditableTileLayer;
class MapRenderer;
class WangFiller;

// Copy of WangId::Index, for exposing the enum to JS
namespace WangIndex
{
    Q_NAMESPACE

    enum Value {
        Top         = 0,
        TopRight    = 1,
        Right       = 2,
        BottomRight = 3,
        Bottom      = 4,
        BottomLeft  = 5,
        Left        = 6,
        TopLeft     = 7,

        NumCorners  = 4,
        NumEdges    = 4,
        NumIndexes  = 8,
    };
    Q_ENUM_NS(Value)
}

class TileLayerWangEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Tiled::EditableTileLayer *target READ target CONSTANT)
    Q_PROPERTY(Tiled::EditableWangSet *wangSet READ wangSet CONSTANT)
    Q_PROPERTY(bool mergeable READ isMergeable WRITE setMergeable)
    Q_PROPERTY(bool correctionsEnabled READ correctionsEnabled WRITE setCorrectionsEnabled)
    Q_PROPERTY(bool erasingEnabled READ erasingEnabled WRITE setErasingEnabled)

public:
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

    bool correctionsEnabled() const;
    void setCorrectionsEnabled(bool correctionsEnabled);

    bool erasingEnabled() const;
    void setErasingEnabled(bool erasingEnabled);

    EditableTileLayer *target() const;
    EditableWangSet *wangSet() const;

    Q_INVOKABLE Tiled::EditableTileLayer *generate();

public slots:
    void setWangIndex(int x, int y, Tiled::WangIndex::Value index, int color);
    void setWangIndex(QPoint pos, Tiled::WangIndex::Value index, int color);
    void setCorner(int x, int y, int color);
    void setCorner(QPoint pos, int color);
    void setEdge(int x, int y, Tiled::WangIndex::Value edge, int color);
    void setEdge(QPoint pos, Tiled::WangIndex::Value edge, int color);

    void apply();

private:
    EditableTileLayer *mTargetLayer;
    EditableWangSet *mWangSet;
    bool mMergeable = false;
    const Map mMap;                             // Copy for the configuration
    std::unique_ptr<MapRenderer> mRenderer;
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

inline EditableWangSet *TileLayerWangEdit::wangSet() const
{
    return mWangSet;
}

inline void TileLayerWangEdit::setWangIndex(int x, int y, WangIndex::Value index, int color)
{
    setWangIndex(QPoint(x, y), index, color);
}

inline void TileLayerWangEdit::setCorner(int x, int y, int color)
{
    setCorner(QPoint(x, y), color);
}

inline void TileLayerWangEdit::setEdge(int x, int y, WangIndex::Value edge, int color)
{
    setEdge(QPoint(x, y), edge, color);
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::TileLayerWangEdit*)
