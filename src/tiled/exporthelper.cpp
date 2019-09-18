/*
 * exporthelper.cpp
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

#include "exporthelper.h"

#include "mapobject.h"
#include "objectgroup.h"

namespace Tiled {

/**
 * @return the format options that should be used when writing the file.
 */
FileFormat::Options ExportHelper::formatOptions() const
{
    FileFormat::Options options;
    if (mOptions.testFlag(Preferences::ExportMinimized))
        options |= FileFormat::WriteMinimized;
    return options;
}

/**
 * Prepares a tileset for export.
 *
 * \a savingTileset means that this tileset is being saved to its own file
 * rather saved as part of a map. In this case, we want to apply some export
 * options that we would skip for external tilesets.
 */
SharedTileset ExportHelper::prepareExportTileset(const SharedTileset &tileset,
                                                 bool savingTileset) const
{
    if (!mOptions)
        return tileset;

    // When the tileset is embedded we're effectively always saving it
    savingTileset |= !tileset->isExternal();

    if (!savingTileset && !mOptions.testFlag(Preferences::EmbedTilesets))
        return tileset; // Leave external tileset alone

    if (savingTileset && !(mOptions & (Preferences::DetachTemplateInstances |
                                       Preferences::ResolveObjectTypesAndProperties))) {
        // We're saving this tileset as-is, so leave it alone
        return tileset;
    }

    // Either needs to be embedded or is already embedded and we may need to
    // make other changes to the tileset
    SharedTileset exportTileset = tileset->clone();
    exportTileset->setOriginalTileset(tileset);

    if (mOptions.testFlag(Preferences::DetachTemplateInstances)) {
        for (Tile *tile : exportTileset->tiles()) {
            if (!tile->objectGroup())
                continue;

            for (MapObject *object : *tile->objectGroup())
                if (object->isTemplateInstance())
                    object->detachFromTemplate();
        }
    }

    if (mOptions.testFlag(Preferences::ResolveObjectTypesAndProperties)) {
        for (Tile *tile : exportTileset->tiles()) {
            if (!tile->objectGroup())
                continue;

            for (MapObject *object : *tile->objectGroup())
                resolveTypeAndProperties(object);
        }
    }

    return exportTileset;
}

const Map *ExportHelper::prepareExportMap(const Map *map, std::unique_ptr<Map> &exportMap) const
{
    // If no export options are active, return the same map
    if (!mOptions)
        return map;

    // Make a copy to which export options are applied
    exportMap = map->clone();

    if (mOptions.testFlag(Preferences::DetachTemplateInstances))
        for (Layer *layer : exportMap->objectGroups())
            for (MapObject *object : *static_cast<ObjectGroup*>(layer))
                if (object->isTemplateInstance())
                    object->detachFromTemplate();

    if (mOptions.testFlag(Preferences::ResolveObjectTypesAndProperties))
        for (Layer *layer : exportMap->objectGroups())
            for (MapObject *object : *static_cast<ObjectGroup*>(layer))
                resolveTypeAndProperties(object);

    auto tilesets = exportMap->tilesets();
    for (const SharedTileset &tileset : tilesets) {
        auto exportTileset = prepareExportTileset(tileset, false);
        if (exportTileset != tileset)
            exportMap->replaceTileset(tileset, exportTileset);
    }

    // Return a pointer to the copy
    return exportMap.get();
}

void ExportHelper::resolveTypeAndProperties(MapObject *object) const
{
    Tile *tile = object->cell().tile();

    // Inherit type from tile if not set on object (not inheriting
    // type from tile of tile object template here, for that the
    // "Detach templates" option needs to be used as well)
    if (object->type().isEmpty() && tile &&
            (!object->isTemplateInstance() || object->propertyChanged(MapObject::CellProperty)))
        object->setType(tile->type());

    Properties properties;

    // Inherit properties from type
    if (!object->type().isEmpty()) {
        for (int i = Object::objectTypes().size() - 1; i >= 0; --i) {
            auto const &type = Object::objectTypes().at(i);
            if (type.name == object->type())
                properties.merge(type.defaultProperties);
        }
    }

    // Inherit properties from tile
    if (tile)
        properties.merge(tile->properties());

    // Override with own properties
    properties.merge(object->properties());

    object->setProperties(properties);
}

} // namespace Tiled
