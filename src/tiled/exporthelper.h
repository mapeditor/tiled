/*
 * exporthelper.h
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

#include "fileformat.h"
#include "preferences.h"
#include "tileset.h"

#include <memory>

namespace Tiled {

/**
 * Applies certain export options to a map and its tilesets, or to a specific
 * tileset.
 */
class ExportHelper
{
public:
    explicit ExportHelper(Preferences::ExportOptions options = Preferences::instance()->exportOptions())
        : mOptions(options)
    {}

    FileFormat::Options formatOptions() const;

    SharedTileset prepareExportTileset(const SharedTileset &tileset,
                                       bool savingTileset = true) const;

    const Map *prepareExportMap(const Map *map, std::unique_ptr<Map> &exportMap) const;

private:
    void resolveTypeAndProperties(MapObject *object) const;

    const Preferences::ExportOptions mOptions;
};

} // namespace Tiled
