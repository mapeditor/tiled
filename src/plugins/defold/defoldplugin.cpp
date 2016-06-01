#include "defoldplugin.h"

#include "tokendefines.h"


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
    QVariantHash map_h;
    map_h["tile_set"] = map->tilesets()[0]->fileName();
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
        layer_h["isVisible"] = tileLayer->isVisible();
        QString cells;
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
                cell_h["h_flip"] = cell.flippedHorizontally;
                cell_h["v_flip"] = cell.flippedVertically;

                Mustache::QtVariantContext context_cell(cell_h);
                cells.append(renderer.render(cell_t, &context_cell));
            }
        }
        layer_h["cell"] = cells;
        Mustache::QtVariantContext context_layer(layer_h);
        layers.append(renderer.render(layers_t, &context_layer);
    }
    Mustache::QtVariantContext context_map(map_h);


    QFile writeFile(fileName);
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        mError = tr("Could not open file for writing.");
        return false;
    }

    writeFile.write(renderer.render(map_t, &context_map));

    if (writeFile.error() != QFile::NoError)
    {
        mError = writeFile.errorString();
        return false;
    }


    return true;
}
