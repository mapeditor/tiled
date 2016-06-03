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
    map_h["tile_set"] = map->tilesets()[0]->fileName().utf16();
    int layerZ = 0;
    QString layers;
    foreach (const Tiled::Layer *layer, map->layers())
    {
        if (layer->layerType() != Tiled::Layer::TileLayerType)
            continue;

        const Tiled::TileLayer *tileLayer = static_cast<const Tiled::TileLayer*>(layer);
        QVariantHash  layer_h;
        layer_h["id"] = tileLayer->name();
        layer_h["z"] = layerZ++;
        layer_h["is_visible"] = tileLayer->isVisible() ? 1 : 0;
        QString cells = "";
        for (int y = 0; y < tileLayer->height(); ++y)
        {
            for (int x = 0; x < tileLayer->width(); ++x)
            {
                const Tiled::Cell &cell = tileLayer->cellAt(x, y);
                if (cell.tile == nullptr) continue;
                QVariantHash cell_h;
                cell_h["x"] = x;
                cell_h["y"] = y;
                cell_h["tile"] = cell.tile->id();
                cell_h["h_flip"] = cell.flippedHorizontally ? 1 : 0;
                cell_h["v_flip"] = cell.flippedVertically ? 1 : 0;

                cells.append(ReplaceTags(cell_t, cell_h));
            }
        }
        layer_h["cells"] = cells;
        layers.append(ReplaceTags(layer_t, layer_h));
        qDebug() << layers;

    }
    map_h["layers"] = layers;
    map_h["material"] = "/builtins/materials/tile_map.material";
    map_h["blend_mode"] = "BLEND_MODE_ALPHA";
    map_h["tile_set"] = "/assets/Terrain/grass/grass.tilesource";

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


    return true;
}
