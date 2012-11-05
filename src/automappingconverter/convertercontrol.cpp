/*
 * convertercontrol.cpp
 * Copyright 2012, Stefan Beller, stefanbeller@googlemail.com
 *
 * This file is part of the AutomappingConverter, which converts old rulemaps
 * of Tiled to work with the latest version of Tiled.
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

#include "convertercontrol.h"

#include "map.h"
#include "layer.h"
#include "tileset.h"

#include "mapreader.h"
#include "mapwriter.h"

#include <QDebug>
#include <QSettings>

using namespace Tiled;

ConverterControl::ConverterControl()
{
}

QString ConverterControl::automappingRuleFileVersion(const QString &fileName)
{
    Tiled::MapReader reader;
    Tiled::Map *map = reader.readMap(fileName);

    if (!map)
        return versionNotAMap();

    // version 1 check
    bool hasonlyruleprefix = true;
    foreach (Tiled::Layer *layer, map->layers()) {
        if (!layer->name().startsWith("rule", Qt::CaseInsensitive))
            hasonlyruleprefix = false;
    }
    if (hasonlyruleprefix)
        return version1();

    // version 2 check
    bool hasrule = false;
    bool hasoutput = false;
    bool hasregion = false;
    bool allused = true;

    foreach (Tiled::Layer *layer, map->layers()) {
        bool isunused = true;
        if (layer->name().startsWith("input", Qt::CaseInsensitive)) {
            hasrule = true;
            isunused = false;
        }
        if (layer->name().startsWith("output", Qt::CaseInsensitive)) {
            hasoutput = true;
            isunused = false;
        }
        if (layer->name().toLower() == "regions") {
            hasregion = true;
            isunused = false;
        }
        if (isunused)
            allused = false;
    }
    if (allused && hasoutput && hasregion && hasrule)
        return version2();

    return versionUnknown();
}

void ConverterControl::convertV1toV2(const QString &fileName)
{
    Tiled::MapReader reader;
    Tiled::Map *map = reader.readMap(fileName);

    if (!map) {
        qWarning() << "Error at conversion of " << fileName << ":\n"
                   << reader.errorString();
        return;
    }

    foreach (Tiled::Layer *layer, map->layers()) {
        if (layer->name().startsWith("ruleset", Qt::CaseInsensitive)) {
            layer->setName("Input_set");
        } else if (layer->name().startsWith("rulenotset", Qt::CaseInsensitive)) {
            layer->setName("InputNot_set");
        } else if (layer->name().startsWith("ruleregions", Qt::CaseInsensitive)) {
            layer->setName("Regions");
        } else if (layer->name().startsWith("rule", Qt::CaseInsensitive)) {
            const QString newname = layer->name().right(layer->name().length() - 4);
            layer->setName("Output" + newname);
        } else {
            qWarning() << QString("Warning at conversion of") << fileName <<
                          QString("unused layers found");
        }
    }

    Tiled::MapWriter writer;
    writer.setLayerDataFormat(map->layerDataFormat());
    writer.writeMap(map, fileName);

    qDeleteAll(map->tilesets());
    delete map;
}
