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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0) // required by QVector::reallocData
    TransformState() = default;
#endif

    explicit TransformState(const MapObject *mapObject)
        : mPosition(mapObject->position())
        , mSize(mapObject->size())
        , mPolygon(mapObject->polygon())
        , mRotation(mapObject->rotation())
        , mChangedProperties(mapObject->changedProperties())
    {}

    bool operator==(const TransformState &o) const
    {
        // not comparing mChangedProperties or mPropertiesChangedNow, because
        // they are not a relevant state difference in themselves.
        return mPosition == o.mPosition &&
               mSize == o.mSize &&
               mPolygon == o.mPolygon &&
               mRotation == o.mRotation;
    }

public:
    QPointF position() const;
    void setPosition(QPointF position);

    QSizeF size() const;
    void setSize(QSizeF size);

    const QPolygonF &polygon() const;
    void setPolygon(const QPolygonF &polygon);

    qreal rotation() const;
    void setRotation(qreal rotation);

    MapObject::ChangedProperties changedProperties() const;
    MapObject::ChangedProperties propertiesChangedNow() const;

private:
    QPointF mPosition;
    QSizeF mSize;
    QPolygonF mPolygon;
    qreal mRotation = 0.0;
    MapObject::ChangedProperties mChangedProperties;
    MapObject::ChangedProperties mPropertiesChangedNow;
};

inline QPointF TransformState::position() const
{
    return mPosition;
}

inline QSizeF TransformState::size() const
{
    return mSize;
}

inline const QPolygonF &TransformState::polygon() const
{
    return mPolygon;
}

inline qreal TransformState::rotation() const
{
    return mRotation;
}

inline MapObject::ChangedProperties TransformState::changedProperties() const
{
    return mChangedProperties;
}

inline MapObject::ChangedProperties TransformState::propertiesChangedNow() const
{
    return mPropertiesChangedNow;
}


class TransformMapObjects : public ChangeValue<MapObject, TransformState>
{
public:
    TransformMapObjects(Document *document,
                        QList<MapObject *> mapObjects,
                        const QVector<TransformState> &states,
                        QUndoCommand *parent = nullptr);

    bool hasAnyChanges() const;

    int id() const override { return Cmd_ChangeMapObjectTransform; }

    void undo() override;
    void redo() override;

    bool mergeWith(const QUndoCommand *other) final;

private:
    TransformState getValue(const MapObject *mapObject) const override;
    void setValue(MapObject *mapObject, const TransformState &value) const override;

    MapObject::ChangedProperties mChangedProperties;
};

inline bool TransformMapObjects::hasAnyChanges() const
{
    return mChangedProperties;
}

} // namespace Tiled
