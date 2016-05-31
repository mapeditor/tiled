#include "defoldplugin.h"



#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

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
    QJsonObject mapJson;
    mapJson["tile_set"] = map->tilesets()[0]->fileName();
    int layerZ = 0;
    foreach (const Tiled::Layer *layer, map->layers())
    {
        if (layer->layerType() != Tiled::Layer::TileLayerType)
            continue;

        const Tiled::TileLayer *tileLayer = static_cast<const Tiled::TileLayer*>(layer);
        QJsonObject  layerJson;
        layerJson["id"] = tileLayer->name();
        layerJson["z"] = layerZ++;
        layerJson["isVisible"] = tileLayer->isVisible();
        QJsonArray cellsArrayJson;
        for (int y = 0; y < tileLayer->height(); ++y)
        {
            for (int x = 0; x < tileLayer->width(); ++x)
            {
                const Tiled::Cell &cell = tileLayer->cellAt(x, y);
                if (cell.tile == nullptr) continue;
                QJsonObject cellJson;
                cellJson["x"] = x;
                cellJson["y"] = y;
                cellJson["tile"] = cell.tile->id();
                cellJson["h_flip"] = cell.flippedHorizontally;
                cellJson["v_flip"] = cell.flippedVertically;
                cellsArrayJson.append(cellJson);
            }
        }
        layerJson["cell"] = cellsArrayJson;

        mapJson["layer"] = layerJson;
    }

    QFile jsonFile(fileName);
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        mError = tr("Could not open file for writing.");
        return false;
    }

    QJsonDocument saveDoc(mapJson);
    jsonFile.write(saveDoc.toJson());

    if (jsonFile.error() != QFile::NoError)
    {
        mError = jsonFile.errorString();
        return false;
    }


    return true;
}
