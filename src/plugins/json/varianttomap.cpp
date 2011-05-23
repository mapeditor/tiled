#include <QtDebug>

#include "jsonplugin.h"
#include "properties.h"
#include "objectgroup.h"
#include "mapobject.h"
#include "tileset.h"
#include "tilelayer.h"
#include "tile.h"

Properties getProperties(QVariantMap oProperties);
Tileset *getTileset(QVariantMap oTileset, int firstGid, QDir mapDir);
TileLayer *getTileLayer(QVariantMap oTileLayer);
ObjectGroup *getObjectGroup(QVariantMap oObjectGroup, Map *map);

QMap<int, const Tileset*> firstGidTileset;
QMap<int, Tileset*> mGidsToTileset;
Tile *tileForGid(int gid, bool &ok);

Map *variantToMap(QVariantMap oMap, QDir mapDir){

    QString orientation=oMap["orientation"].toString();
    Map::Orientation orientationFlag = Map::Unknown;
    if (orientation == "orthogonal") {
        orientationFlag = Map::Orthogonal;
    } else if (orientation == "isometric") {
        orientationFlag = Map::Isometric;
    }

    Map *map =new Map(orientationFlag,
                      oMap["width"].toInt(),
                      oMap["height"].toInt(),
                      oMap["tileWidth"].toInt(),
                      oMap["tileHeight"].toInt());

    map->setProperties(getProperties(oMap["properties"].toMap()));

    QVariantList oTilesets=oMap["tilesets"].toList();
    firstGidTileset.clear();
    int firstGid = 1;
    foreach (const QVariant o, oTilesets) {
        QVariantMap oTileset=o.toMap();
        Tileset *tileset =getTileset( oTileset, firstGid, mapDir);
        map->addTileset(tileset);
        firstGidTileset.insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }

    QVariantList oLayers=oMap["layers"].toList();
    foreach (const QVariant l, oLayers) {
        QVariantMap oLayer=l.toMap();
        Layer *layer;
        if (oLayer["type"]=="Layer"){
            layer=getTileLayer(oLayer);
        } else if (oLayer["type"]=="ObjectGroup"){
            layer=getObjectGroup(oLayer, map);
        }
        layer->setProperties(getProperties(oLayer["properties"].toMap()));
        map->addLayer(layer);
    }

    mGidsToTileset.clear();
    return map;
}

Properties getProperties(QVariantMap oProperties){
    Properties properties;
    if (!oProperties.isEmpty()) {
        QVariantMap::const_iterator it = oProperties.constBegin();
        QVariantMap::const_iterator it_end = oProperties.constEnd();
        for (; it != it_end; ++it) {
            properties[it.key()]=it.value().toString();
        }
    }
    return properties;
}

Tileset *getTileset(QVariantMap oTileset, int firstGid, QDir mapDir){
    Tileset *tileset=new Tileset(oTileset["name"].toString(),
                                 oTileset["tileWidth"].toInt(),
                                 oTileset["tileHeight"].toInt(),
                                 oTileset["spacing"].toInt(),
                                 oTileset["margin"].toInt());
    QVariantMap oImage=oTileset["image"].toMap();
    QString imageSource=mapDir.absoluteFilePath(oImage["source"].toString());
    tileset->loadFromImage(QImage(imageSource), imageSource);

    QVariantMap oProperties=oTileset["properties"].toMap();
    QVariantMap::const_iterator it = oProperties.constBegin();
    for (; it != oProperties.constEnd(); ++it) {
        tileset->tileAt(it.key().toInt())->setProperties(getProperties(it.value().toMap()));
    }
    mGidsToTileset.insert(firstGid, tileset);
    return tileset;
}

TileLayer *getTileLayer(QVariantMap oTileLayer){
    TileLayer *tileLayer=new TileLayer(oTileLayer["name"].toString(),
                                       oTileLayer["x"].toInt(),
                                       oTileLayer["y"].toInt(),
                                       oTileLayer["width"].toInt(),
                                       oTileLayer["height"].toInt());
    QVariantList oTiles=oTileLayer["tiles"].toList();

    int x = 0;
    int y = 0;
    foreach (const QVariant t, oTiles) {
        bool ok;
        Tile *tile=tileForGid(t.toInt(), ok);
        tileLayer->setTile(x,y,tile);
        x++;
        if (x >= tileLayer->width()) {
            x = 0;
            y++;
        }
    }

    return tileLayer;
}

ObjectGroup *getObjectGroup(QVariantMap oObjectGroup, Map *map){
    ObjectGroup *objectGroup=new ObjectGroup(oObjectGroup["name"].toString(),
                                             oObjectGroup["x"].toInt(),
                                             oObjectGroup["y"].toInt(),
                                             oObjectGroup["width"].toInt(),
                                             oObjectGroup["height"].toInt());

    QVariantList oObjects=oObjectGroup["objects"].toList();

    objectGroup->setColor(oObjectGroup.value("color").value<QColor>());

    foreach (const QVariant o, oObjects) {
        QVariantMap oObject=o.toMap();

        const QString name = oObject["name"].toString();
        const QString type = oObject["type"].toString();
        const int gid =      oObject["gid"].toInt();
        const int x =        oObject["x"].toInt();
        const int y =        oObject["y"].toInt();
        const int width =    oObject["width"].toInt();
        const int height =   oObject["height"].toInt();

        const int tileHeight = map->tileHeight();
        const int tileWidth = map->tileWidth();
        qreal xF, yF, widthF, heightF;

        if (map->orientation() == Map::Isometric) {
            // Isometric needs special handling, since the pixel values are based
            // solely on the tile height.
            xF = (qreal) x / tileHeight;
            yF = (qreal) y / tileHeight;
            widthF = (qreal) width / tileHeight;
            heightF = (qreal) height / tileHeight;
        } else {
            xF = (qreal) x / tileWidth;
            yF = (qreal) y / tileHeight;
            widthF = (qreal) width / tileWidth;
            heightF = (qreal) height / tileHeight;
        }

        MapObject *object = new MapObject(name, type, xF, yF, widthF, heightF);

        if (gid) {
            bool ok;
            Tile *tile = tileForGid(gid, ok);
            if (ok) {
                object->setTile(tile);
            } else {

            }
        }
        object->setProperties(getProperties(oObject["properties"].toMap()));
        qDebug()<< object->name() << object->type() << object->x() << object->y() << object->width() << object->height();
        objectGroup->addObject(object);
    }

    return objectGroup;
}

Tile *tileForGid(int gid, bool &ok)
{
    Tile *result = 0;

    if (gid < 0) {
        //        xml.raiseError(tr("Invalid global tile id (less than 0): %1").arg(gid));
        ok = false;
    } else if (gid == 0) {
        ok = true;
    } else if (mGidsToTileset.isEmpty()) {
        //        xml.raiseError(tr("Tile used but no tilesets specified"));
        ok = false;
    } else {
        // Find the tileset containing this tile
        QMap<int, Tileset*>::const_iterator i = mGidsToTileset.upperBound(gid);
        --i; // Navigate one tileset back since upper bound finds the next
        const int tileId = gid - i.key();
        const Tileset *tileset = i.value();

        result = tileset ? tileset->tileAt(tileId) : 0;
        ok = true;
    }

    return result;
}

