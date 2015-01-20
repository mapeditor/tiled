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
#include "tile.h"
#include "tilelayer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#if QT_VERSION >= 0x050100
#define HAS_QSAVEFILE_SUPPORT
#endif

#ifdef HAS_QSAVEFILE_SUPPORT
#include <QSaveFile>
#endif

using namespace Tiled;
using namespace Csv;

CsvPlugin::CsvPlugin()
{
}

bool CsvPlugin::write(const Map *map, const QString &fileName)
{
    // Extract file name without extension and path
    QFileInfo fileInfo(fileName);
    fileInfo.setCaching(false);
    const QString fileNameWoExtension = fileInfo.baseName();
    const QString filePath = fileInfo.path();

    // Traverse all tile layers
    foreach (const Layer *layer, map->layers()) {
        if (layer->layerType() != Layer::TileLayerType)
            continue;
            
        // Get the output file name for this layer
        const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
        const QString layerName = tileLayer->name();
        const QString layerFileName = fileNameWoExtension + QString("_") + layerName + QString(".csv");
        const QString layerFileNameWPath = QDir(filePath).filePath(layerFileName);
            
#ifdef HAS_QSAVEFILE_SUPPORT
        QSaveFile file(layerFileNameWPath);
#else
        QFile file(layerFileName);
#endif
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            mError = tr("Could not open file for writing.");
            return false;
        }

        // Write out tiles either by ID or their name, if given. -1 is "empty"
        for (int y = 0; y < tileLayer->height(); ++y) {
            for (int x = 0; x < tileLayer->width(); ++x) {
                if (x > 0)
                    file.write(",", 1);
    
                const Cell &cell = tileLayer->cellAt(x, y);
                const Tile *tile = cell.tile;
                if (tile && tile->hasProperty(QLatin1String("name"))) {
                    file.write(tile->property(QLatin1String("name")).toUtf8());
                } else {
                    const int id = tile ? tile->id() : -1;
                    file.write(QByteArray::number(id));
                }
            }
    
            file.write("\n", 1);
        }
    
        if (file.error() != QFile::NoError) {
            mError = file.errorString();
            return false;
        }

#ifdef HAS_QSAVEFILE_SUPPORT
        if (!file.commit()) {
            mError = file.errorString();
            return false;
        }
#endif
    }
    return true;
}

QString CsvPlugin::nameFilter() const
{
    return tr("CSV files (*.csv)");
}

QString CsvPlugin::errorString() const
{
    return mError;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Csv, CsvPlugin)
#endif
