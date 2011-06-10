#include <QtDebug>
#include <QSettings>

#include "jsonplugin.h"
#include "properties.h"
#include "objectgroup.h"
#include "mapobject.h"
#include "tileset.h"
#include "tilelayer.h"
#include "tile.h"




class VariantToMapHelper{
public:

    VariantToMapHelper(QVariantMap map, QDir mapDir):oMap(map),mapDir(mapDir){

        QString orientation=oMap["orientation"].toString();
        Map::Orientation orientationFlag = Map::Unknown;
        if (orientation == "orthogonal") {
            orientationFlag = Map::Orthogonal;
        } else if (orientation == "isometric") {
            orientationFlag = Map::Isometric;
        }

        tMap =new Map(orientationFlag,
                      oMap["width"].toInt(),
                      oMap["height"].toInt(),
                      oMap["tileWidth"].toInt(),
                      oMap["tileHeight"].toInt());

        tMap->setProperties(getProperties(oMap["properties"].toMap()));

        QVariantList oTilesets=oMap["tilesets"].toList();
        mGidsToTileset.clear();
        int firstGid = 1;
        foreach (const QVariant o, oTilesets) {
            QVariantMap oTileset=o.toMap();
            Tileset *tileset =getTileset( oTileset, firstGid, mapDir);
            tMap->addTileset(tileset);
            mGidsToTileset.insert(firstGid, tileset);
            firstGid += tileset->tileCount();
        }

        QVariantList oLayers=oMap["layers"].toList();
        foreach (const QVariant l, oLayers) {
            QVariantMap oLayer=l.toMap();
            Layer *layer=0;
            if (oLayer["type"]=="Layer"){
                layer=getTileLayer(oLayer);
            } else if (oLayer["type"]=="ObjectGroup"){
                layer=getObjectGroup(oLayer, tMap);
            }
            layer->setProperties(getProperties(oLayer["properties"].toMap()));
            tMap->addLayer(layer);
        }

        mGidsToTileset.clear();
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
        if (oTileLayer["encoding"]=="base64") {
            QString text=oTileLayer["data"].toString();

#if QT_VERSION < 0x040800
            const QString textData = QString::fromRawData(text.unicode(), text.size());
            const QByteArray latin1Text = textData.toLatin1();
#else
            const QByteArray latin1Text = text.toLatin1();
#endif
            QByteArray tileData = QByteArray::fromBase64(latin1Text);
            const int size = (tileLayer->width() * tileLayer->height()) * 4;

            if (size != tileData.length()) {
//                xml.raiseError(tr("Corrupt layer data for layer '%1'")
//                               .arg(tileLayer->name()));
                return tileLayer;
            }

            const unsigned char *data =
                    reinterpret_cast<const unsigned char*>(tileData.constData());
            int x = 0;
            int y = 0;

            for (int i = 0; i < size - 3; i += 4) {
                const uint gid = data[i] |
                        data[i + 1] << 8 |
                                       data[i + 2] << 16 |
                                       data[i + 3] << 24;

                bool ok;
                Cell cell = cellForGid(gid, ok);
                if (ok)
                    tileLayer->setCell(x, y, cell);
                else {
//                    xml.raiseError(tr("Invalid tile: %1").arg(gid));
                    return tileLayer;
                }

                x++;
                if (x == tileLayer->width()) {
                    x = 0;
                    y++;
                }
            }



        }else{
            QVariantList oTiles=oTileLayer["tiles"].toList();

            int x = 0;
            int y = 0;
            foreach (const QVariant t, oTiles) {
                bool ok;
                Cell cell=cellForGid(t.toInt(),ok);
                tileLayer->setCell(x,y,cell);
                x++;
                if (x >= tileLayer->width()) {
                    x = 0;
                    y++;
                }
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
                Cell cell=cellForGid(gid,ok);
                if (ok) {
                    object->setTile(cell.tile);
                } else {

                }
            }
            object->setProperties(getProperties(oObject["properties"].toMap()));
            //        qDebug()<< object->name() << object->type() << object->x() << object->y() << object->width() << object->height();
            objectGroup->addObject(object);
        }

        return objectGroup;
    }


    Cell cellForGid(uint gid, bool &ok)
    {
        Cell result;

        if (gid == 0) {
            ok = true;
        } else if (mGidsToTileset.isEmpty()) {
            ok = false;
        } else {
            // Read out the flags
            result.flippedHorizontally = (gid & FlippedHorizontallyFlag);
            result.flippedVertically = (gid & FlippedVerticallyFlag);

            // Clear the flags
            gid &= ~(FlippedHorizontallyFlag | FlippedVerticallyFlag);

            // Find the tileset containing this tile
            QMap<uint, Tileset*>::const_iterator i = mGidsToTileset.upperBound(gid);
            --i; // Navigate one tileset back since upper bound finds the next
            const int tileId = gid - i.key();
            const Tileset *tileset = i.value();

            result.tile = tileset ? tileset->tileAt(tileId) : 0;
            ok = true;
        }

        return result;
    }

    Map *map(){
        return tMap;
    }

protected:
    Map *tMap;
    QVariantMap oMap;
    QDir mapDir;

    QMap<uint, Tileset*> mGidsToTileset;
};

Map *variantToMap(QVariantMap oMap, QDir mapDir){
    return VariantToMapHelper(oMap,mapDir).map();
}
