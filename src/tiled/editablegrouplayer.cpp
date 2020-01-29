/*
 * editablegrouplayer.cpp
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

#include "editablegrouplayer.h"

#include "addremovelayer.h"
#include "editablemanager.h"
#include "editablemap.h"
#include "scriptmanager.h"

#include <QCoreApplication>

namespace Tiled {

EditableGroupLayer::EditableGroupLayer(const QString &name, QObject *parent)
    : EditableLayer(std::unique_ptr<Layer>(new GroupLayer(name, 0, 0)), parent)
{
}

EditableGroupLayer::EditableGroupLayer(EditableMap *map, GroupLayer *groupLayer, QObject *parent)
    : EditableLayer(map, groupLayer, parent)
{
}

EditableLayer *EditableGroupLayer::layerAt(int index)
{
    if (index < 0 || index >= layerCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return nullptr;
    }

    Layer *layer = groupLayer()->layerAt(index);
    return EditableManager::instance().editableLayer(map(), layer);
}

void EditableGroupLayer::removeLayerAt(int index)
{
    if (index < 0 || index >= layerCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    if (asset())
        asset()->push(new RemoveLayer(mapDocument(), index, groupLayer()));
    else if (!checkReadOnly())
        EditableManager::instance().release(groupLayer()->takeLayerAt(index));
}

void EditableGroupLayer::removeLayer(EditableLayer *editableLayer)
{
    if (!editableLayer) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid argument"));
        return;
    }

    int index = groupLayer()->layers().indexOf(editableLayer->layer());
    if (index == -1) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Layer not found"));
        return;
    }

    removeLayerAt(index);
}

void EditableGroupLayer::insertLayerAt(int index, EditableLayer *editableLayer)
{
    if (index < 0 || index > layerCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    if (!editableLayer) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid argument"));
        return;
    }

    if (!editableLayer->isOwning()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Layer is in use"));
        return;
    }

    if (asset()) {
        asset()->push(new AddLayer(mapDocument(), index, editableLayer->layer(), groupLayer()));
    } else if (!checkReadOnly()) {
        // ownership moves to the group layer
        groupLayer()->insertLayer(index, editableLayer->release());
    }
}

void EditableGroupLayer::addLayer(EditableLayer *editableLayer)
{
    insertLayerAt(layerCount(), editableLayer);
}

} // namespace Tiled
