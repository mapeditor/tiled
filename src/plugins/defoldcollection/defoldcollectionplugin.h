/*
 * DefoldCollection Tiled Plugin
 * Copyright 2019, CodeSpartan
 * Based on Defold Tiled Plugin by Nikita Razdobreev and Thorbjørn Lindeijer
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

#include "defoldcollectionplugin_global.h"
#include "tiled.h"

#include "mapformat.h"

namespace DefoldCollection {

class DEFOLDCOLLECTIONPLUGINSHARED_EXPORT DefoldCollectionPlugin : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    DefoldCollectionPlugin();

    bool write(const Tiled::Map *map, const QString &collectionFile, Options options) override;
    QString errorString() const override;
    QString shortName() const override;

protected:
    QString nameFilter() const override;

private:
    QString mError;
};

} // namespace DefoldCollection
