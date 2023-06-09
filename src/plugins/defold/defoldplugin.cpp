/*
 * Defold Tiled Plugin
 * Copyright 2016, Nikita Razdobreev <exzo0mex@gmail.com>
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "defoldplugin.h"

#include "layer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <QTextStream>

#include <cmath>

namespace Defold {

static const char cell_t[] =
"  cell {\n\
    x: {{x}}\n\
    y: {{y}}\n\
    tile: {{tile}}\n\
    h_flip: {{h_flip}}\n\
    v_flip: {{v_flip}}\n\
    rotate90: {{rotate90}}\n\
  }\n";

static const char layer_t[] =
"layers {\n\
  id: \"{{id}}\"\n\
  z: {{z}}\n\
  is_visible: {{is_visible}}\n\
{{cells}}\
}\n";

static const char map_t[] =
"tile_set: \"{{tile_set}}\"\n\
{{layers}}\n\
material: \"{{material}}\"\n\
blend_mode: {{blend_mode}}\n";

static QString replaceTags(QString context, const QVariantHash &map)
{
    QHashIterator<QString,QVariant> it{map};
    while (it.hasNext()) {
        it.next();
        context.replace(QLatin1String("{{") + it.key() + QLatin1String("}}"),
                        it.value().toString());
    }
    return context;
}

static void setCellProperties(QVariantHash &cellHash, const Tiled::Cell &cell)
{
    cellHash["tile"] = cell.tileId();

    if (cell.flippedAntiDiagonally()) {
        cellHash["h_flip"] = cell.flippedVertically() ? 1 : 0;
        cellHash["v_flip"] = cell.flippedHorizontally() ? 0 : 1;
        cellHash["rotate90"] = 1;
    } else {
        cellHash["h_flip"] = cell.flippedHorizontally() ? 1 : 0;
        cellHash["v_flip"] = cell.flippedVertically() ? 1 : 0;
        cellHash["rotate90"] = 0;
    }
}

template <typename T>
static T optionalProperty(const Tiled::Object *object, const QString &name, const T &def)
{
    const QVariant var = object->resolvedProperty(name);
    return var.isValid() ? var.value<T>() : def;
}

DefoldPlugin::DefoldPlugin()
{
}

QString DefoldPlugin::nameFilter() const
{
    return tr("Defold Tile Map (*.tilemap)");
}

QString DefoldPlugin::shortName() const
{
    return QStringLiteral("defold");
}

QString DefoldPlugin::errorString() const
{
    return mError;
}

bool DefoldPlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)

    QVariantHash map_h;

    QString layers;
    Tiled::LayerIterator it(map, Tiled::Layer::TileLayerType);
    double z = 0.0;

    while (auto tileLayer = static_cast<Tiled::TileLayer*>(it.next())) {
        // Defold exports the z value to be between -1 and 1, so these
        // automatic increments should allow up to 10000 layers.
        z = optionalProperty(tileLayer, QStringLiteral("z"), z + 0.0001);

        QVariantHash layer_h;
        layer_h["id"] = tileLayer->name();
        layer_h["z"] = z;
        layer_h["is_visible"] = tileLayer->isVisible() ? 1 : 0;
        QString cells;

        for (int x = 0; x < tileLayer->width(); ++x) {
            for (int y = 0; y < tileLayer->height(); ++y) {
                const Tiled::Cell &cell = tileLayer->cellAt(x, y);
                if (cell.isEmpty())
                    continue;
                QVariantHash cell_h;
                cell_h["x"] = x;
                cell_h["y"] = tileLayer->height() - y - 1;
                setCellProperties(cell_h, cell);
                cells.append(replaceTags(QLatin1String(cell_t), cell_h));
            }
        }
        layer_h["cells"] = cells;
        layers.append(replaceTags(QLatin1String(layer_t), layer_h));
    }
    map_h["layers"] = layers;
    map_h["material"] = "/builtins/materials/tile_map.material";
    map_h["blend_mode"] = "BLEND_MODE_ALPHA";
    map_h["tile_set"] = map->property(QStringLiteral("tile_set")).toString();

    QString result = replaceTags(QLatin1String(map_t), map_h);
    Tiled::SaveFile mapFile(fileName);
    if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }
    QTextStream stream(mapFile.device());
    stream << result;

    if (mapFile.error() != QFileDevice::NoError) {
        mError = mapFile.errorString();
        return false;
    }

    if (!mapFile.commit()) {
        mError = mapFile.errorString();
        return false;
    }

    return true;
}

} // namespace Defold
