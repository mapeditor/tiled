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
#include "wangset.h"

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
    const bool hasExportSettings = !(tileset->exportFileName.isEmpty()
                                     && tileset->exportFormat.isEmpty());

    if (!mOptions && !hasExportSettings)
        return tileset;

    // When the tileset is embedded we're effectively always saving it
    savingTileset |= !tileset->isExternal();

    if (!savingTileset && !mOptions.testFlag(Preferences::EmbedTilesets))
        return tileset; // Leave external tileset alone

    if (savingTileset && !(mOptions & (Preferences::DetachTemplateInstances |
                                       Preferences::ResolveObjectTypesAndProperties))
            && !hasExportSettings) {
        // We're saving this tileset as-is, so leave it alone
        return tileset;
    }

    // Either needs to be embedded or is already embedded and we may need to
    // make other changes to the tileset
    SharedTileset exportTileset = tileset->clone();
    exportTileset->setOriginalTileset(tileset);

    // We don't want to save the export options in the exported file
    if (hasExportSettings) {
        exportTileset->exportFileName.clear();
        exportTileset->exportFormat.clear();
    }

    if (mOptions.testFlag(Preferences::DetachTemplateInstances)) {
        for (Tile *tile : exportTileset->tiles()) {
            if (!tile->objectGroup())
                continue;

            for (MapObject *object : *tile->objectGroup())
                if (object->isTemplateInstance())
                    object->detachFromTemplate();
        }
    }

    if (mOptions.testFlag(Preferences::ResolveObjectTypesAndProperties))
        resolveProperties(exportTileset.data());

    return exportTileset;
}

const Map *ExportHelper::prepareExportMap(const Map *map, std::unique_ptr<Map> &exportMap) const
{
    const bool hasExportSettings = !(map->exportFileName.isEmpty()
                                     && map->exportFormat.isEmpty());

    // If no export options are active, return the same map
    if (!(mOptions & ~Preferences::ExportMinimized) && !hasExportSettings)
        return map;

    // Make a copy to which export options are applied
    exportMap = map->clone();

    // We don't want to save the export options in the exported file
    if (hasExportSettings) {
        exportMap->exportFileName.clear();
        exportMap->exportFormat.clear();
    }

    if (mOptions.testFlag(Preferences::DetachTemplateInstances)) {
        for (Layer *layer : exportMap->objectGroups()) {
            for (MapObject *object : *static_cast<ObjectGroup*>(layer)) {
                if (object->isTemplateInstance()) {
                    // In case of templated tile objects, the map may not yet
                    // have a reference to the used tileset.
                    if (Tile *tile = object->cell().tile())
                        exportMap->addTileset(tile->tileset()->sharedFromThis());

                    object->detachFromTemplate();
                }
            }
        }
    }

    if (mOptions.testFlag(Preferences::ResolveObjectTypesAndProperties))
        resolveProperties(exportMap.get());

    const auto tilesets = exportMap->tilesets();    // needs a copy
    for (const SharedTileset &tileset : tilesets) {
        auto exportTileset = prepareExportTileset(tileset, false);
        if (exportTileset != tileset)
            exportMap->replaceTileset(tileset, exportTileset);
    }

    // Return a pointer to the copy
    return exportMap.get();
}

static bool resolveClassPropertyMembers(QVariant &value)
{
    if (value.userType() != propertyValueId())
        return false;

    auto propertyValue = value.value<PropertyValue>();
    const PropertyType *propertyType = propertyValue.type();
    if (!propertyType || !propertyType->isClass())
        return false;

    auto classType = static_cast<const ClassPropertyType*>(propertyType);
    QVariantMap classValue = propertyValue.value.toMap();
    bool changed = false;

    // iterate over the members of the class type, making sure each
    // member is present in classValue, recursively resolving its members
    auto it = classType->members.begin();
    const auto it_end = classType->members.end();
    for (; it != it_end; ++it) {
        const auto &memberName = it.key();
        auto &value = classValue[memberName];

        if (!value.isValid()) {
            value = it.value();
            changed = true;
        }

        changed |= resolveClassPropertyMembers(value);
    }

    if (changed) {
        propertyValue.value = classValue;
        value = QVariant::fromValue(propertyValue);
    }

    return changed;
}

static void resolveClassPropertyMembers(QVariantMap &properties)
{
    for (auto &value : properties)
        resolveClassPropertyMembers(value);
}

void ExportHelper::resolveProperties(Object *object) const
{
    switch (object->typeId()) {
    case Object::MapObjectType: {
        // Map objects need special handling because:
        //
        // * We don't want to inherit the properties from the template, since
        //   that is covered by a separate "Detach templates" option.
        //
        // * They can inherit their class from their tile (again, unless that
        //   tile came from a template).
        //
        auto mapObject = static_cast<MapObject*>(object);
        auto tile = mapObject->cell().tile();

        if (mapObject->className().isEmpty() && tile &&
                (!mapObject->isTemplateInstance() ||
                 mapObject->propertyChanged(MapObject::CellProperty))) {
            mapObject->setClassName(tile->className());
        }

        Properties properties;

        // Inherit properties from the class
        if (auto type = Object::propertyTypes().findClassFor(mapObject->className(), *mapObject))
            mergeProperties(properties, type->members);

        // Inherit properties from the tile
        if (tile)
            mergeProperties(properties, tile->properties());

        // Override with own properties
        mergeProperties(properties, mapObject->properties());

        resolveClassPropertyMembers(properties);
        mapObject->setProperties(properties);
        return;
    }
    case Object::LayerType:
        if (static_cast<Layer*>(object)->isObjectGroup()) {
            auto objectGroup = static_cast<ObjectGroup*>(object);
            for (MapObject *mapObject : *objectGroup)
                resolveProperties(mapObject);
        }
        // Group layers are handled by layer iterator
        break;
    case Object::MapType:
        for (auto layer : static_cast<Map*>(object)->allLayers())
            resolveProperties(layer);
        // Tilesets are handled by prepareExportTileset
        break;
    case Object::TilesetType:
        for (auto tile : static_cast<Tileset*>(object)->tiles())
            resolveProperties(tile);
        for (auto wangSet : static_cast<Tileset*>(object)->wangSets())
            resolveProperties(wangSet);
        break;
    case Object::TileType:
        if (auto objectGroup = static_cast<Tile*>(object)->objectGroup())
            resolveProperties(objectGroup);
        break;
    case Object::WangSetType:
        for (const auto &color : static_cast<WangSet*>(object)->colors())
            resolveProperties(color.data());
        break;
    case Object::WangColorType:
    case Object::ProjectType:
    case Object::WorldType:
        break;
    }

    auto properties = object->resolvedProperties();
    resolveClassPropertyMembers(properties);
    object->setProperties(properties);
}

} // namespace Tiled
