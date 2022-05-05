/*
 * transformmapobjects.h
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#include "mapobject.h"
#include "undocommands.h"

namespace Tiled {

class Document;

struct TransformState
{
    explicit TransformState(const MapObject *mapObject)
        : mPosition(mapObject->position())
        , mSize(mapObject->size())
        , mRotation(mapObject->rotation())
        , mChangedProperties(mapObject->changedProperties())
    {}

    bool operator==(const TransformState &o) const
    {
        return mPosition == o.mPosition &&
               mSize == o.mSize &&
               mRotation == o.mRotation &&
               mChangedProperties == o.mChangedProperties;
    }

public:
    QPointF position() const;
    void setPosition(QPointF position);

    QSizeF size() const;
    void setSize(QSizeF size);

    qreal rotation() const;
    void setRotation(qreal rotation);

    MapObject::ChangedProperties changedProperties() const;

private:
    QPointF mPosition;
    QSizeF mSize;
    qreal mRotation;
    MapObject::ChangedProperties mChangedProperties;
};

inline QPointF TransformState::position() const
{
    return mPosition;
}

inline QSizeF TransformState::size() const
{
    return mSize;
}

inline qreal TransformState::rotation() const
{
    return mRotation;
}

inline MapObject::ChangedProperties TransformState::changedProperties() const
{
    return mChangedProperties;
}


class TransformMapObjects : public ChangeValue<MapObject, TransformState>
{
public:
    TransformMapObjects(Document *document,
                        QList<MapObject *> mapObjects,
                        const QVector<TransformState> &states);

    int id() const override { return Cmd_ChangeMapObjectTransform; }

    void undo() override;
    void redo() override;

private:
    TransformState getValue(const MapObject *mapObject) const override;
    void setValue(MapObject *mapObject, const TransformState &value) const override;
};

} // namespace Tiled
