/*
 * editabletilelayer.h
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

#include "editablelayer.h"
#include "regionvaluetype.h"
#include "tilelayer.h"

namespace Tiled {

class EditableTile;
class TileLayerEdit;

class EditableTileLayer : public EditableLayer
{
    Q_OBJECT

    Q_PROPERTY(int width READ width WRITE setWidth)
    Q_PROPERTY(int height READ height WRITE setHeight)
    Q_PROPERTY(QSize size READ size WRITE setSize)

public:
    Q_INVOKABLE explicit EditableTileLayer(const QString &name = QString(),
                                           QSize size = QSize(0, 0),
                                           QObject *parent = nullptr);

    explicit EditableTileLayer(EditableMap *map,
                               TileLayer *layer,
                               QObject *parent = nullptr);
    ~EditableTileLayer() override;

    int width() const;
    int height() const;
    QSize size() const;

    void setWidth(int width);
    void setHeight(int height);
    void setSize(QSize size);

    Q_INVOKABLE void resize(QSize size, QPoint offset = QPoint());

    Q_INVOKABLE Tiled::RegionValueType region() const;

    Q_INVOKABLE Tiled::Cell cellAt(int x, int y) const;
    Q_INVOKABLE int flagsAt(int x, int y) const;
    Q_INVOKABLE Tiled::EditableTile *tileAt(int x, int y) const;

    Q_INVOKABLE Tiled::TileLayerEdit *edit();

    TileLayer *tileLayer() const;

private:
    friend TileLayerEdit;

    QList<TileLayerEdit*> mActiveEdits;
};


inline int EditableTileLayer::width() const
{
    return tileLayer()->width();
}

inline int EditableTileLayer::height() const
{
    return tileLayer()->height();
}

inline QSize EditableTileLayer::size() const
{
    return tileLayer()->size();
}

inline void EditableTileLayer::setWidth(int width)
{
    setSize(QSize(width, height()));
}

inline void EditableTileLayer::setHeight(int height)
{
    setSize(QSize(width(), height));
}

inline TileLayer *EditableTileLayer::tileLayer() const
{
    return static_cast<TileLayer*>(layer());
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableTileLayer*)
