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

#include "grouplayer.h"
#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

using namespace Tiled;
using namespace Csv;

// Bits on the far end of the 32-bit tile ID are used for tile flags
const unsigned FlippedHorizontallyFlag   = 0x80000000;
const unsigned FlippedVerticallyFlag     = 0x40000000;
const unsigned FlippedAntiDiagonallyFlag = 0x20000000;
const unsigned RotatedHexagonal120Flag   = 0x10000000;

CsvPlugin::CsvPlugin()
{
}

bool CsvPlugin::write(const Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)

    // Get file paths for each layer
    const QStringList layerPaths = outputFiles(map, fileName);

    // Traverse all tile layers
    int currentLayer = 0;
    for (const Layer *layer : map->tileLayers()) {
        const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);

        SaveFile file(layerPaths.at(currentLayer));

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
            mError += QLatin1String("\n");
            mError += layerPaths.at(currentLayer);
            return false;
        }

        auto device = file.device();

        QRect bounds = map->infinite() ? tileLayer->region().boundingRect() : tileLayer->rect();
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
                    int id = -1;

                    if (tile) {
                        id = tile->id();

                        if (cell.flippedHorizontally())
                            id |= FlippedHorizontallyFlag;
                        if (cell.flippedVertically())
                            id |= FlippedVerticallyFlag;
                        if (cell.flippedAntiDiagonally())
                            id |= FlippedAntiDiagonallyFlag;
                        if (cell.rotatedHexagonal120())
                            id |= RotatedHexagonal120Flag;
                    }

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
    const QRegularExpression reservedChars(QStringLiteral("[<>:\"/\\|?*]"));

    QStringList result;

    // Extract file name without extension and path
    QFileInfo fileInfo(fileName);
    const QString base = fileInfo.completeBaseName();
    const QDir dir = fileInfo.dir();

    // Loop layers to calculate the path for the exported file
    for (const Layer *layer : map->tileLayers()) {
        // Get the output file name for this layer
        QString layerNames;

        do {
            layerNames.prepend(layer->name());
            layerNames.prepend(QLatin1Char('_'));
        } while ((layer = layer->parentLayer()));

        layerNames.replace(reservedChars, QStringLiteral("_"));

        const QString layerFileName = base + layerNames + QLatin1String(".csv");
        const QString layerFilePath = dir.filePath(layerFileName);

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
    return QStringLiteral("csv");
}
