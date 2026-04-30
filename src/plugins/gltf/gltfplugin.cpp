/*
 * glTF Tiled Plugin
 * Copyright 2026, Prateek Singh <prateeksingh765000017@gmail.com>
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

#include "gltfplugin.h"

#include "gltfexporter.h"

using namespace Gltf;

GltfPlugin::GltfPlugin()
{
}

bool GltfPlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    mError.clear();
    const GltfExporter exporter;
    return exporter.write(map, fileName, options, &mError);
}

QString GltfPlugin::errorString() const
{
    return mError;
}

QStringList GltfPlugin::outputFiles(const Tiled::Map *map, const QString &fileName) const
{
    Q_UNUSED(map)
    return GltfExporter::outputFiles(fileName);
}

QString GltfPlugin::nameFilter() const
{
    return tr("glTF files (*.gltf *.glb)");
}

QString GltfPlugin::shortName() const
{
    return QStringLiteral("gltf");
}
