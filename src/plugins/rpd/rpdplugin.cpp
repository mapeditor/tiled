/*
 * Remixed Dungeon Tiled Plugin
 * Copyright 2025, Mikhael Danilov <mikhael.danilov@gmail.com>
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

#include "rpdplugin.h"

#include <logginginterface.h>

#include "maptovariantconverter.h"
#include "objectgroup.h"
#include "savefile.h"
#include "varianttomapconverter.h"

#include "qjsonparser/json.h"


#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QtMath>


namespace Rpd {

const QString LAYER_LOGIC = "logic";
const QString LAYER_BASE = "base";
const QString LAYER_DECO = "deco";
const QString LAYER_DECO2 = "deco2";
const QString LAYER_ROOF_BASE = "roof_base";
const QString LAYER_ROOF_DECO = "roof_deco";
const QString LAYER_OBJECTS = "objects";

const QString PROPERTY_TILES = "tiles";
const QString PROPERTY_WATER = "water";
const QString PROPERTY_COMPASS_TARGET = "compassTarget";

const QString OBJECT_CLASS_MOB = "mob";
const QString OBJECT_CLASS_ITEM = "item";
const QString OBJECT_CLASS_OBJECT = "object";

void RpdPlugin::initialize()
{
    addObject(new RpdMapFormat(this));
    addObject(new RpdTilesetFormat(this));
}

RpdMapFormat::RpdMapFormat(QObject *parent)
    : Tiled::WritableMapFormat(parent)
{}

QString RpdMapFormat::shortName() const
{
    return "rpd";
}

static QJsonArray packMapData(Tiled::TileLayer *layer)
{
    QJsonArray map;
    for (int j = 0; j < layer->map()->height(); ++j)
        for (int i = 0; i < layer->map()->width(); ++i)
            map.append(layer->cellAt(i, j).tileId());
    return map;
}

bool RpdMapFormat::insertTilesetFile(Tiled::Layer &layer,
                                     const QString &tiles_name,
                                     QJsonObject &mapJson)
{
    const auto tilesets = layer.usedTilesets();

    if (tilesets.isEmpty()) {
        mError = tr("You have an empty %1 layer, please fill it").arg(layer.name());
        Tiled::ERROR(mError);
        return false;
    }

    if (tilesets.size() > 1) {
        mError = tr("Only one tileset per layer supported (%1 layer) ->\n").arg(layer.name());
        for (const auto &tileset : tilesets)
            mError += "[" + tileset->name() + "]\n";
        Tiled::ERROR(mError);
        return false;
    }

    mapJson.insert(tiles_name, tilesets.begin()->data()->name() + ".png");
    return true;
}

bool RpdMapFormat::validateMap(const Tiled::Map *map) {
    bool haveLogicLayer = false;

    const QList<QString> tileLayers = {LAYER_LOGIC, LAYER_BASE, LAYER_DECO, LAYER_DECO2, LAYER_ROOF_BASE, LAYER_ROOF_DECO};
    const QList<QString> objectLayers = {LAYER_OBJECTS};

    for (Tiled::Layer *layer : map->layers()) {
        auto layerName = layer->name();

        if (layer->isTileLayer()) {
            if(layerName == LAYER_LOGIC) {
                haveLogicLayer = true;
                continue;
            }
            if(!tileLayers.contains(layerName)) {
                Tiled::WARNING(tr("You have an unknown tile layer (%1), it will be ignored\n").arg(layerName));
            }
        }

        if (layer->isObjectGroup()) {
            if(!objectLayers.contains(layerName)) {
                Tiled::WARNING(tr("You have an unknown object layer (%1), it will be ignored\n").arg(layerName));
            }
        }
    }

    if (!haveLogicLayer) {
        mError = tr("You must have a layer with tile layer type and name 'logic'");
        Tiled::ERROR(mError);
        return false;
    }

    return true;
}

void RpdMapFormat::validateAndWriteProperties(const Tiled::Map *map, QJsonObject &mapJson) {

    const QList<QString> knownStringProperties = {PROPERTY_TILES, PROPERTY_WATER, PROPERTY_COMPASS_TARGET};

    for (const auto &[key, value] : map->properties().toStdMap()) {
        if (value.canConvert<double>()) {
            mapJson.insert(key, value.toDouble());
            continue;
        }

        if (value.canConvert<QString>()) {
            if(!knownStringProperties.contains(key)) {
                Tiled::WARNING(tr("Don't know about property (%1) it probably will be ignored").arg(key));
            }

            auto jsonCandidate = QJsonDocument::fromJson(value.toString().toUtf8());

            if (jsonCandidate.isObject()) {
                mapJson.insert(key, jsonCandidate.object());
                continue;
            }

            if(jsonCandidate.isArray()) {
                mapJson.insert(key, jsonCandidate.array());
                continue;
            }

            mapJson.insert(key, value.toString());
            continue;
        }

        Tiled::WARNING(tr("Don't know what to do with property (%1) it will be ignored").arg(key));
    }

    if (!mapJson.contains(PROPERTY_TILES))
        mapJson.insert(PROPERTY_TILES, "tiles0_x.png");

    if(!mapJson.contains(PROPERTY_WATER))
        mapJson.insert(PROPERTY_WATER, "water0.png");
}

bool RpdMapFormat::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    if (!validateMap(map)) {
        return false;
    }

    Tiled::SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    QJsonObject mapJson;

    validateAndWriteProperties(map, mapJson);

    for (Tiled::Layer *layer : map->layers()) {
        auto layerName = layer->name();

        if (layer->isTileLayer()) {
            if (!insertTilesetFile(*layer, QString("tiles_") + layer->name(), mapJson)) {
                return false;
            }

            auto tileLayer = layer->asTileLayer();

            if (layerName == LAYER_LOGIC) {
                if (!writeLogicLayer(mapJson, tileLayer))
                    return false;
            }

            if (layerName == LAYER_BASE)
                mapJson.insert("baseTileVar", packMapData(tileLayer));

            if (layerName == LAYER_DECO2)
                mapJson.insert("deco2TileVar", packMapData(tileLayer));

            if (layerName == LAYER_ROOF_BASE)
                mapJson.insert("roofBaseTileVar", packMapData(tileLayer));

            if (layerName == LAYER_ROOF_DECO)
                mapJson.insert("roofDecoTileVar", packMapData(tileLayer));

            if (layerName == LAYER_DECO) {
                if (!writeDecoLayer(mapJson, tileLayer))
                    return false;
            }
        }

        if (layer->isObjectGroup()) {
            auto objectLayer = layer->asObjectGroup();
            if (layerName == LAYER_OBJECTS) {
                writeObjectLayer(mapJson, objectLayer);
            }
        }
    }

    JsonWriter writer;
    writer.setAutoFormatting(!options.testFlag(WriteMinimized));

    if (!writer.stringify(QJsonDocument(mapJson).toVariant())) {
        // This can only happen due to coding error
        mError = writer.errorString();
        return false;
    }

    QTextStream out(file.device());
    out << writer.result();

    out.flush();

    if (file.error() != QFileDevice::NoError) {
        mError = tr("Error while writing file:\n%1").arg(file.errorString());
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

void RpdMapFormat::writeObjectLayer(QJsonObject &mapJson, const Tiled::ObjectGroup *objectLayer) {

    const QList <QString> knownObjectsKind = {OBJECT_CLASS_ITEM, OBJECT_CLASS_MOB, OBJECT_CLASS_OBJECT};
    QMap<QString, QJsonArray> objects;

    for (auto object: objectLayer->objects()) {
        QJsonObject desc;

        desc.insert("kind", object->name());

        desc.insert("x", qFloor(object->x() / 16.));
        desc.insert("y", qFloor(object->y() / 16.));

        auto properties = object->properties();
        for (auto i = properties.begin(); i != properties.end(); ++i) {
            auto jsonCandidate = QJsonDocument::fromJson(i.value().toString().toUtf8());

            if (jsonCandidate.isObject()) {
                desc.insert(i.key(), jsonCandidate.object());
                continue;
            }

            desc.insert(i.key(), i.value().toJsonValue());
        }

        if(!knownObjectsKind.contains(object->type())) {
            Tiled::WARNING(tr("Don't know about object class (%1) it probably will be ignored").arg(object->type()));
        }

        objects[object->type()].append(desc);
    }

    for (const auto &key: objects.keys())
        mapJson.insert(key + "s", objects[key]);

}

bool RpdMapFormat::writeDecoLayer(QJsonObject &mapJson, Tiled::TileLayer *layer) {
    mapJson.insert("decoTileVar", packMapData(layer));
    mapJson.insert("customTiles", true);

    if (!insertTilesetFile(*layer, "tiles", mapJson))
        return false;

    QJsonArray decoDesc;
    QJsonArray decoName;

    auto tilesets = layer->usedTilesets();

    auto decoTileset = tilesets.begin()->data();

    auto it = decoTileset->tiles().begin();
    auto end = decoTileset->tiles().end();

    while (it != end) {
        decoDesc.append(((*it)->properties())["deco_desc"].toString());
        decoName.append(((*it)->properties())["deco_name"].toString());
        ++it;
    }

    mapJson.insert("decoName", decoName);
    mapJson.insert("decoDesc", decoDesc);
    return true;
}

bool RpdMapFormat::writeLogicLayer(QJsonObject &mapJson, Tiled::TileLayer *layer) {
    QJsonArray entrance;
    QJsonArray multiexit;

    mapJson.insert("width", layer->map()->width());
    mapJson.insert("height", layer->map()->height());

    mapJson.insert("map", packMapData(layer));

    bool isOk = true;

    for (int i = 0; i < layer->map()->width(); ++i) {
        for (int j = 0; j < layer->map()->height(); ++j) {
            int tileId = layer->cellAt(i, j).tileId();

            if (tileId < 0) {
                mError = tr("Hole in logic layer at (%1, %2)").arg(i).arg(j);
                Tiled::ERROR(mError);
                isOk = false;
            }

            switch (tileId) {
                case ENTRANCE:
                    entrance.append(i);
                    entrance.append(j);
                    break;
                case EXIT:
                case LOCKED_EXIT:
                case UNLOCKED_EXIT: {
                    QJsonArray exit;
                    exit.append(i);
                    exit.append(j);
                    multiexit.append(exit);
                } break;
                default:
                    break;
            }
        }
    }

    mapJson.insert("entrance", entrance);
    mapJson.insert("multiexit", multiexit);

    return isOk;
}

QString RpdMapFormat::nameFilter() const
{
    return tr("Remixed Pixel Dungeon levels (*.json)");
}

QString RpdMapFormat::errorString() const
{
    return mError;
}

RpdTilesetFormat::RpdTilesetFormat(QObject *parent)
    : Tiled::TilesetFormat(parent)
{}

Tiled::SharedTileset RpdTilesetFormat::read(const QString &fileName)
{
    Q_UNUSED(fileName)
    return Tiled::SharedTileset();
}

bool RpdTilesetFormat::supportsFile(const QString &fileName) const
{
    Q_UNUSED(fileName)
    return false;
}

bool RpdTilesetFormat::write(const Tiled::Tileset &tileset, const QString &fileName, Options options)
{
    Tiled::SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    Tiled::MapToVariantConverter converter;
    QVariant variant = converter.toVariant(tileset, QFileInfo(fileName).dir());

    JsonWriter writer;
    writer.setAutoFormatting(!options.testFlag(WriteMinimized));

    if (!writer.stringify(variant)) {
        // This can only happen due to coding error
        mError = writer.errorString();
        return false;
    }

    QTextStream out(file.device());
    out << writer.result();
    out.flush();

    if (file.error() != QFileDevice::NoError) {
        mError = tr("Error while writing file:\n%1").arg(file.errorString());
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString RpdTilesetFormat::nameFilter() const
{
    return tr("Json tileset files (*.json)");
}

QString RpdTilesetFormat::errorString() const
{
    return mError;
}

QString RpdTilesetFormat::shortName() const
{
    return "RPD";
}

} // namespace Rpd
