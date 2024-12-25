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
#include "addremovetileset.h"
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

QList<QObject *> EditableGroupLayer::layers()
{
    QList<QObject *> editables;
    auto editableMap = map();

    for (const auto layer : groupLayer()->layers())
        editables.append(EditableLayer::get(editableMap, layer));

    return editables;
}

EditableLayer *EditableGroupLayer::layerAt(int index)
{
    if (index < 0 || index >= layerCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return nullptr;
    }

    Layer *layer = groupLayer()->layerAt(index);
    return EditableLayer::get(map(), layer);
}

void EditableGroupLayer::removeLayerAt(int index)
{
    if (index < 0 || index >= layerCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    if (MapDocument *doc = mapDocument())
        asset()->push(new RemoveLayer(doc, index, groupLayer()));
    else if (!checkReadOnly())
        EditableLayer::release(groupLayer()->takeLayerAt(index));
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
        ScriptManager::instance().throwNullArgError(1);
        return;
    }

    if (!editableLayer->isOwning()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Layer is in use"));
        return;
    }

    const auto tilesets = editableLayer->layer()->usedTilesets();

    if (MapDocument *doc = mapDocument()) {
        auto command = new AddLayer(doc, index, editableLayer->layer(), groupLayer());

        for (const auto &tileset : tilesets)
            if (!doc->map()->tilesets().contains(tileset))
                new AddTileset(doc, tileset, command);

        asset()->push(command);
    } else if (!checkReadOnly()) {
        if (auto map = groupLayer()->map())
            map->addTilesets(tilesets);

        // ownership moves to the group layer
        groupLayer()->insertLayer(index, editableLayer->attach(asset()));
    }
}

void EditableGroupLayer::addLayer(EditableLayer *editableLayer)
{
    if (!editableLayer) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    insertLayerAt(layerCount(), editableLayer);
}

} // namespace Tiled

#include "moc_editablegrouplayer.cpp"
