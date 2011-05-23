#include "jsonplugin.h"

#include "json.h"
#include "map.h"
#include "tileset.h"
#include <QtDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

JsonPlugin::JsonPlugin()
{
}

Tiled::Map *JsonPlugin::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        mError = tr("Could not open file for reading.");
        return false;
    }

    QVariantMap oMap=Json::parse(file.readAll(), &mError).toMap();
    if (oMap.empty())
        mError =tr("Error parsing file.");
    if (mError!="")
        return false;
    return variantToMap(oMap, QFileInfo(fileName).dir());
}

bool JsonPlugin::write(const Tiled::Map *map, const QString &fileName)
{

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        mError = tr("Could not open file for writing.");
        return false;
    }
    QTextStream out(&file);

    out << Json::stringify(mapToVariant(map,QFileInfo(fileName).dir()));

//    QVariantMap nG=Json::parse(source).toMap();
//    qDebug() << nG["tilesets"].toList()[0].toMap()["imageSource"];
    return true;
}

QString JsonPlugin::nameFilter() const
{
    return tr("Json files (*.json)");
}

bool JsonPlugin::supportsFile(const QString &fileName) const
{
    return fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive);
}

QString JsonPlugin::errorString() const
{
    return mError;
}

Q_EXPORT_PLUGIN2(Json, JsonPlugin)







