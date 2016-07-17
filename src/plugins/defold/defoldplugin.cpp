/*
 * Defold Tiled Plugin
 * Copyright 2016, Nikita Razdobreev <exzo0mex@gmail.com>
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

#include "tokendefines.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTextStream>
#include <QDebug>
#include <tilelayer.h>
#include <mapobject.h>
#include <objectgroup.h>

#include <algorithm>
#include <cmath>

#include "map.h"
#include "tile.h"
#include "tilelayer.h"
#include "layer.h"

QStringList Defold::DefoldPlugin::outputFiles(const Tiled::Map *map, const QString &fileName) const
{
    QFileInfo fileInfo(fileName);
    const QString base = fileInfo.completeBaseName() + QLatin1String("_");
    QString path = fileName;
    return QStringList(path);
}

QString Defold::DefoldPlugin::ReplaceTags(QString context, QVariantHash map)
{
    QString str = context;
    foreach(QString key, map.keys())
    {
        QStringList str_list = str.split("{{" + key +"}}");
        if (str_list.length() == 1)  str = str_list[0];
        if (str_list.length() == 2)  str = str_list[0] + map[key].toString() + str_list[1];
    }
    return str;
}

QString Defold::DefoldPlugin::nameFilter() const
{
    return tr("Defold files (*.tilemap)");
}
QString Defold::DefoldPlugin::errorString() const
{
    return mError;
}

Defold::DefoldPlugin::DefoldPlugin()
{

}

bool Defold::DefoldPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    QVariantHash map_h;

    QList<QList<QString>> types;

    int layerWidth = 0;
    int layerHeight = 0;

    int cellWidth = 0;
    int cellHeight = 0;

    QString layers = "";
    foreach (Tiled::TileLayer *tileLayer, map->tileLayers())
    {
        QVariantHash  layer_h;
        layer_h["id"] = tileLayer->name();
        layer_h["z"] = 0;
        layer_h["is_visible"] = tileLayer->isVisible() ? 1 : 0;
        QString cells = "";

        layerWidth = std::max(tileLayer->width(), layerWidth);
        layerHeight = std::max(tileLayer->height(), layerHeight);

        for (int x = 0; x < tileLayer->width(); ++x)
        {
            QList<QString> t;
            if (types.size() < tileLayer->width()) types.append(t);
            for (int y = 0; y < tileLayer->height(); ++y)
            {
             const Tiled::Cell &cell = tileLayer->cellAt(x, y);
                if (cell.tile == nullptr) continue;
                QVariantHash cell_h;
                cell_h["x"] = x;
                cell_h["y"] = tileLayer->height() - y - 1;
                cell_h["tile"] = cell.tile->id();
                cell_h["h_flip"] = cell.flippedHorizontally ? 1 : 0;
                cell_h["v_flip"] = cell.flippedVertically ? 1 : 0;
                cells.append(ReplaceTags(cell_t, cell_h));
                if (types[x].size() < tileLayer->height())
                    types[x].append(cell.tile->property("Type").toString());
                else if (!cell.tile->property("Type").toString().isEmpty())
                    types[x][tileLayer->height() - y - 1] = cell.tile->property("Type").toString();
                cellWidth = std::max(cell.tile->width(), cellWidth);
                cellHeight = std::max(cell.tile->height(), cellHeight);
            }
        }
        layer_h["cells"] = cells;
        layers.append(ReplaceTags(layer_t, layer_h));
    }
    map_h["layers"] = layers;
    map_h["material"] = "/builtins/materials/tile_map.material";
    map_h["blend_mode"] = "BLEND_MODE_ALPHA";
    map_h["tile_set"] = "";

    QString result = ReplaceTags(map_t, map_h);
    QFile writeFile(fileName);
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        mError = tr("Could not open file for writing.");
        return false;
    }
    QTextStream  stream (&writeFile);
    stream << result;

    if (writeFile.error() != QFile::NoError)
    {
        mError = writeFile.errorString();
        return false;
    }

    QFile saveFile(fileName + ".script");
    if (!saveFile.open(QIODevice::WriteOnly))
    {
            qWarning("Couldn't open save file.");
            return false;
    }

    QTextStream  unitsFileStream (&saveFile);
    unitsFileStream << "map_nodes ={" << endl;

    for (int y = 0; y < layerHeight; ++y)
    {

        unitsFileStream <<"{";
        for (int x = 0; x < layerWidth; ++x)
        {
            unitsFileStream << "\t" << types[x][y] << ",";
        }
        unitsFileStream <<"}," << endl;
    }


    unitsFileStream << "}" << endl;

    unitsFileStream << "Objects={" << endl;
    foreach (Tiled::ObjectGroup *group, map->objectGroups())
    {
        foreach (Tiled::MapObject *object, group->objects())
        {
            unitsFileStream << "\t{" << endl;
            QString name = object->name();
            QPointF pos = object->position();
            unitsFileStream << "\tname=\"" <<  name << "\"," << endl;
            unitsFileStream << "\tx="  << std::floor((float)pos.x() / (cellWidth / 2.0f)) * cellWidth / 2 << "," << endl;
            unitsFileStream << "\ty=" <<  std::floor((float)pos.y() / (cellHeight / 2.0f)) * cellHeight / 2 << "," << endl;
            unitsFileStream << "\t}," << endl;
        }
    }
    unitsFileStream << "}" << endl;
    return true;
}
