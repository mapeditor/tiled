/*
 * editableselectedarea.h
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "regionvaluetype.h"

#include <QObject>
#include <QRect>

class QUndoCommand;

namespace Tiled {

class MapDocument;

class EditableSelectedArea : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QRect boundingRect READ boundingRect)

public:
    explicit EditableSelectedArea(MapDocument *mapDocument,
                                  QObject *parent = nullptr);

    QRect boundingRect() const;

    Q_INVOKABLE Tiled::RegionValueType get() const;

public slots:
    void set(const QRect &rect);
    void set(const QRectF &rect);
    void set(const Tiled::RegionValueType &region);
    void add(const QRect &rect);
    void add(const QRectF &rect);
    void add(const Tiled::RegionValueType &region);
    void subtract(const QRect &rect);
    void subtract(const QRectF &rect);
    void subtract(const Tiled::RegionValueType &region);
    void intersect(const QRect &rect);
    void intersect(const QRectF &rect);
    void intersect(const Tiled::RegionValueType &region);

private:
    void set(const QRegion &region);

    MapDocument * const mMapDocument;
};


inline QRect EditableSelectedArea::boundingRect() const
{
    return get().boundingRect();
}

inline void EditableSelectedArea::set(const QRectF &rect)
{
    set(rect.toRect());
}

inline void EditableSelectedArea::add(const QRectF &rect)
{
    add(rect.toRect());
}

inline void EditableSelectedArea::subtract(const QRectF &rect)
{
    subtract(rect.toRect());
}

inline void EditableSelectedArea::intersect(const QRectF &rect)
{
    intersect(rect.toRect());
}

} // namespace Tiled
