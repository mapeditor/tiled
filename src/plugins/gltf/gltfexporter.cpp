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

#include "gltfexporter.h"

#include "map.h"

#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>

using namespace Gltf;

GltfExporter::ContainerFormat GltfExporter::containerFormat(const QString &fileName)
{
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    if (suffix == QLatin1String("gltf"))
        return ContainerFormat::Gltf;
    if (suffix == QLatin1String("glb"))
        return ContainerFormat::Glb;
    return ContainerFormat::Unknown;
}

QStringList GltfExporter::outputFiles(const QString &fileName)
{
    QStringList files;
    files << fileName;

    // A text .gltf writes vertex/index data to a sibling .bin file. A binary
    // .glb embeds it, so only the single file is produced. Enumerating these
    // lets Tiled's "export as" UI warn before overwriting the .bin.
    if (containerFormat(fileName) == ContainerFormat::Gltf) {
        QFileInfo info(fileName);
        files << info.absolutePath() + QLatin1Char('/') + info.completeBaseName() + QLatin1String(".bin");
    }

    return files;
}

bool GltfExporter::write(const Tiled::Map *map,
                         const QString &fileName,
                         Tiled::MapFormat::Options options,
                         QString *error) const
{
    Q_UNUSED(map)
    Q_UNUSED(fileName)
    Q_UNUSED(options)

    // The actual writer is the subject of the GSoC 2026 proposal
    // (https://github.com/mapeditor/tiled/issues/2741). This scaffold
    // registers the format and name filter so the plumbing can be reviewed
    // separately from the exporter logic.
    if (error)
        *error = QObject::tr("glTF export is not implemented yet (GSoC 2026 work in progress).");
    return false;
}
