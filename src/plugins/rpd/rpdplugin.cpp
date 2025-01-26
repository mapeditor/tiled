#include "rpdplugin.h"

#include "maptovariantconverter.h"
#include "varianttomapconverter.h"
#include "savefile.h"
#include "objectgroup.h"

#include "qjsonparser/json.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QtMath>

namespace Rpd {

void RpdPlugin::initialize()
{
    addObject(new RpdMapFormat(RpdMapFormat::Rpd, this));
    addObject(new RpdTilesetFormat(this));
}


RpdMapFormat::RpdMapFormat(SubFormat subFormat, QObject *parent)
    : mSubFormat(subFormat)
{
    (void)parent;
}

QString RpdMapFormat::shortName(void) const
{
    return "RPD";
}

QJsonArray packMapData(Tiled::Layer *layer)
{
    QJsonArray map;
    for(int j=0;j<layer->map()->height();++j){
        for(int i=0;i<layer->map()->width();++i){
            map.append(layer->asTileLayer()->cellAt(i,j).tileId());
        }
    }
    return map;
}

bool RpdMapFormat::insertTilesetFile(Tiled::Layer &layer, const QString &tiles_name, QJsonObject &mapJson)
{
    auto tilesets = layer.asTileLayer()->usedTilesets();

    if(tilesets.size()==0) {
        QString msg = QString("You have ")+layer.name()+" layer please fill it";
        mError = tr(msg.toUtf8());
        return false;
    }

    if(tilesets.size()>1) {
        QString msg = QString("Only one tileset per layer supported (")+layer.name()+" layer) ->\n";
        for(auto tileset: tilesets) {
            msg += "[" + tileset->name() + "]\n";
        }
        mError = tr(msg.toUtf8());
        return false;
    }

    mapJson.insert(tiles_name,tilesets.begin()->data()->name()+".png");
    return true;
}




bool RpdMapFormat::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    (void)fileName;
    (void)options;
    Tiled::SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    QJsonObject mapJson;

    for(const auto& property : map->properties().toStdMap()) {
        QVariant value = property.second;
        if(value.canConvert<double>()) {
           mapJson.insert(property.first, value.toDouble());
           continue;
        }

        if(value.canConvert<QString>()) {
            mapJson.insert(property.first, value.toString());
            continue;
        }

        mError = tr("Dont know what to do with property (%1)").arg(property.first);
        return false;
    }

    for(Tiled::Layer *layer: map->layers()) {

        if(layer->layerType()==Tiled::Layer::TileLayerType && !insertTilesetFile(*layer,QString("tiles_")+layer->name(),mapJson)) {
            return false;
        }

        if(layer->name()=="logic") {

            QJsonArray entrance;
            QJsonArray multiexit;

            mapJson.insert("width",layer->map()->width());
            mapJson.insert("height",layer->map()->height());

            mapJson.insert("map",packMapData(layer));

            for(int i=0;i<layer->map()->width();++i){
                for(int j=0;j<layer->map()->height();++j){
                    int tileId = layer->asTileLayer()->cellAt(i,j).tileId();

                    if(tileId<0) {
                        mError = tr("Hole in logic layer at (%1, %2)").arg(i).arg(j);
                        return false;
                    }

                    switch (tileId) {
                    case TileId::ENTRANCE:
                        entrance.append(i);
                        entrance.append(j);
                        break;
                    case TileId::EXIT:
                    case TileId::LOCKED_EXIT:
                    case TileId::UNLOCKED_EXIT:
                    {
                        QJsonArray exit;
                        exit.append(i);
                        exit.append(j);
                        multiexit.append(exit);
                    }
                        break;
                    }
                }
            }

            mapJson.insert("entrance",entrance);
            mapJson.insert("multiexit",multiexit);
        }

        if(layer->name() == "base") {
            mapJson.insert("baseTileVar",packMapData(layer));
        }

        if(layer->name() == "deco2") {
            mapJson.insert("deco2TileVar",packMapData(layer));
        }

        if(layer->name() == "roof_base") {
            mapJson.insert("roofBaseTileVar",packMapData(layer));
        }

        if(layer->name() == "roof_deco") {
            mapJson.insert("roofDecoTileVar",packMapData(layer));
        }

        if(layer->name() == "deco") {
            mapJson.insert("decoTileVar",packMapData(layer));
            mapJson.insert("customTiles",true);

            if(!insertTilesetFile(*layer,"tiles",mapJson)) {
                return false;
            }

            QJsonArray decoDesc;
            QJsonArray decoName;

            auto tilesets = layer->asTileLayer()->usedTilesets();

            auto decoTileset = tilesets.begin()->data();

            auto it = decoTileset->tiles().begin();
            auto end = decoTileset->tiles().end();

            while (it != end) {
                decoDesc.append(((*it)->properties())["deco_desc"].toString());
                decoName.append(((*it)->properties())["deco_name"].toString());
                ++it;
            }

            mapJson.insert("decoName",decoName);
            mapJson.insert("decoDesc",decoDesc);
        }

        if(layer->name()=="objects") {

            QMap<QString, QJsonArray> objects;

            for (auto object : layer->asObjectGroup()->objects()) {

                QJsonObject desc;

                desc.insert("kind",object->name());

                desc.insert("x",qFloor(object->x()/16.));
                desc.insert("y",qFloor(object->y()/16.));

                auto properties = object->properties();
                for (auto i = properties.begin(); i != properties.end(); ++i) {

                    auto jsonCandidate = QJsonDocument::fromJson(i.value().toString().toUtf8());

                    if(jsonCandidate.isObject()) {
                        desc.insert(i.key(),jsonCandidate.object());
                        continue;
                    }

                    desc.insert(i.key(),i.value().toJsonValue());
                }

                objects[object->type()].append(desc);
            }

            for (auto key : objects.keys() ) {
                mapJson.insert(key + "s", objects[key]);
            }
        }
    }

    if(!mapJson.contains("tiles")){
        mapJson.insert("tiles","tiles0_x.png");
    }

    mapJson.insert("water","water0.png");

    QJsonDocument mapDoc;
    mapDoc.setObject(mapJson);

    QVariant variant = mapDoc.toVariant();

    JsonWriter writer;
    writer.setAutoFormatting(true);

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
{
}

Tiled::SharedTileset RpdTilesetFormat::read(const QString &fileName)
{
    (void)fileName;
    return Tiled::SharedTileset();
}

bool RpdTilesetFormat::supportsFile(const QString &fileName) const
{
    (void)fileName;
    return false;
}

bool RpdTilesetFormat::write(const Tiled::Tileset &tileset,
                              const QString &fileName,
                              Options options)
{
    (void)fileName;
    (void)options;
    Tiled::SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    Tiled::MapToVariantConverter converter;
    QVariant variant = converter.toVariant(tileset, QFileInfo(fileName).dir());

    JsonWriter writer;
    writer.setAutoFormatting(true);

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

QString RpdTilesetFormat::shortName(void) const
{
    return "RPD";
}

} // namespace Rpd
