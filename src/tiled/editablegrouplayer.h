/*
 * editablegrouplayer.h
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

#include "editablelayer.h"
#include "grouplayer.h"

namespace Tiled {

class EditableGroupLayer : public EditableLayer
{
    Q_OBJECT

    Q_PROPERTY(int layerCount READ layerCount)

public:
    Q_INVOKABLE explicit EditableGroupLayer(const QString &name = QString(),
                                            QObject *parent = nullptr);

    EditableGroupLayer(EditableMap *map,
                       GroupLayer *groupLayer,
                       QObject *parent = nullptr);

    int layerCount() const;

    Q_INVOKABLE Tiled::EditableLayer *layerAt(int index);
    Q_INVOKABLE void removeLayerAt(int index);
    Q_INVOKABLE void removeLayer(Tiled::EditableLayer *editableLayer);
    Q_INVOKABLE void insertLayerAt(int index, Tiled::EditableLayer *editableLayer);
    Q_INVOKABLE void addLayer(Tiled::EditableLayer *editableLayer);

private:
    GroupLayer *groupLayer() const;
};

inline int EditableGroupLayer::layerCount() const
{
    return groupLayer()->layerCount();
}

inline GroupLayer *EditableGroupLayer::groupLayer() const
{
    return static_cast<GroupLayer*>(layer());
}

} // namespace Tiled
