/*
 * CSV Tiled Plugin
 * Copyright 2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "csvplugin.h"

#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QDir>
#include <QFileInfo>

using namespace Tiled;
using namespace Csv;

CsvPlugin::CsvPlugin()
{
}

bool CsvPlugin::write(const Map *map, const QString &fileName)
{
    // Get file paths for each layer
    QStringList layerPaths = outputFiles(map, fileName);

    // Traverse all tile layers
    int currentLayer = 0;
    for (const Layer *layer : map->layers()) {
        if (layer->layerType() != Layer::TileLayerType)
            continue;

        const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);

        SaveFile file(layerPaths.at(currentLayer));

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            mError = tr("Could not open file for writing.");
            return false;
        }

        auto device = file.device();

        QRect bounds = map->infinite() ? tileLayer->bounds() : tileLayer->rect();
        bounds.translate(-layer->position());

        // Write out tiles either by ID or their name, if given. -1 is "empty"
        for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
            for (int x = bounds.left(); x <= bounds.right(); ++x) {
                if (x > bounds.left())
                    device->write(",", 1);

                const Cell &cell = tileLayer->cellAt(x, y);
                const Tile *tile = cell.tile();
                if (tile && tile->hasProperty(QLatin1String("name"))) {
                    device->write(tile->property(QLatin1String("name")).toString().toUtf8());
                } else {
                    const int id = tile ? tile->id() : -1;
                    device->write(QByteArray::number(id));
                }
            }

            device->write("\n", 1);
        }

        if (file.error() != QFileDevice::NoError) {
            mError = file.errorString();
            return false;
        }

        if (!file.commit()) {
            mError = file.errorString();
            return false;
        }

        ++currentLayer;
    }
    return true;
}

QString CsvPlugin::errorString() const
{
    return mError;
}

QStringList CsvPlugin::outputFiles(const Tiled::Map *map, const QString &fileName) const
{
    QStringList result;

    // Extract file name without extension and path
    QFileInfo fileInfo(fileName);
    const QString base = fileInfo.completeBaseName() + QLatin1String("_");
    const QString path = fileInfo.path();

    // Loop layers to calculate the path for the exported file
    for (const Layer *layer : map->layers()) {
        if (layer->layerType() != Layer::TileLayerType)
            continue;

        // Get the output file name for this layer
        const QString layerName = layer->name();
        const QString layerFileName = base + layerName + QLatin1String(".csv");
        const QString layerFilePath = QDir(path).filePath(layerFileName);

        result.append(layerFilePath);
    }

    // If there was only one tile layer, there's no need to change the name
    // (also keeps behavior backwards compatible)
    if (result.size() == 1)
        result[0] = fileName;

    return result;
}

QString CsvPlugin::nameFilter() const
{
    return tr("CSV files (*.csv)");
}

QString CsvPlugin::shortName() const
{
    return QLatin1String("csv");
}
