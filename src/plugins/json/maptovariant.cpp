#include <QtDebug>
#include <QDir>
#include <QSettings>

#include "jsonplugin.h"
#include "properties.h"
#include "objectgroup.h"
#include "mapobject.h"
#include "tileset.h"
#include "tilelayer.h"
#include "tile.h"

class MapToVariantHelper{
public:
    MapToVariantHelper(const Map *map, QDir mapDir):tMap(map), mapDir(mapDir){

        QString orientation;
        switch (tMap->orientation()) {
        case Map::Orthogonal:
            orientation = QLatin1String("orthogonal");
            break;
        case Map::Isometric:
            orientation = QLatin1String("isometric");
            break;
        case Map::Hexagonal:
            orientation = QLatin1String("hexagonal");
            break;
        case Map::Unknown:
            break;
        }

        oMap["version"]=1.0;
        if (!orientation.isEmpty())
            oMap["orientation"]=orientation;
        oMap["width"]=tMap->width();
        oMap["height"]=tMap->height();
        oMap["tileWidth"]=tMap->tileWidth();
        oMap["tileHeight"]=tMap->tileHeight();

        oMap["properties"]=getProperties(tMap->properties());

        QVariantList oTilesets;

        mFirstGidToTileset.clear();
        int firstGid = 1;
        foreach (const Tileset *tileset, tMap->tilesets()) {
            oTilesets << getTileset( tileset, firstGid);
            mFirstGidToTileset.insert(firstGid, tileset);
            firstGid += tileset->tileCount();
        }
        oMap["tilesets"]=oTilesets;

        QVariantList oLayers;
        foreach (const Layer *layer, tMap->layers()) {
            const TileLayer *tileLayer=dynamic_cast<const TileLayer*>(layer);
            const ObjectGroup *objectGroup=dynamic_cast<const ObjectGroup *>(layer);
            if (tileLayer != 0)
                oLayers << getTileLayer(tileLayer);
            else if (objectGroup != 0)
                oLayers << getObjectGroup(objectGroup);
        }
        oMap["layers"]=oLayers;
    }

    QVariantMap getTileset(const Tileset *tileset, int firstGid)
    {
        QVariantMap oTileset;
        if (firstGid > 0)
            oTileset["firstgid"]=firstGid;

        oTileset["name"]=tileset->name();
        oTileset["tileWidth"]=tileset->tileWidth();
        oTileset["tileHeight"]=tileset->tileHeight();
        oTileset["tileCount"]=tileset->tileCount();

        const int tileSpacing = tileset->tileSpacing();
        const int margin = tileset->margin();
        if (tileSpacing != 0)
            oTileset["spacing"]=tileSpacing;
        if (margin != 0)
            oTileset["margin"]=margin;

        // Write the image element
        const QString &imageSource = tileset->imageSource();
        if (!imageSource.isEmpty()) {
            QVariantMap oImage;
            oImage["source"]=mapDir.relativeFilePath(tileset->imageSource());

            const QColor transColor = tileset->transparentColor();
            if (transColor.isValid())
                oImage["trans"]=transColor.name().mid(1);

            if (tileset->imageWidth() > 0)
                oImage["width"]=tileset->imageWidth();
            if (tileset->imageHeight() > 0)
                oImage["height"]=tileset->imageHeight();

            oTileset["image"]=oImage;
        }

        // Write the properties for those tiles that have them
        QVariantMap oProperties;
        for (int i = 0; i < tileset->tileCount(); ++i) {
            const Tile *tile = tileset->tileAt(i);
            const Properties properties = tile->properties();
            if (!properties.isEmpty()) {
                oProperties[QString::number(i)]=getProperties(properties);
            }
        }
        if (!oProperties.empty()){
            oTileset["properties"]=oProperties;
        }

        return oTileset;
    }

    QVariantMap getProperties(const Properties &properties)
    {
        QVariantMap oProperties;
        if (!properties.isEmpty()) {
            Properties::const_iterator it = properties.constBegin();
            Properties::const_iterator it_end = properties.constEnd();
            for (; it != it_end; ++it) {
                oProperties[it.key()]=it.value();
            }
        }
        return oProperties;
    }

    QVariantMap getTileLayer(const TileLayer *tileLayer)
    {
        QSettings s;
        QVariantMap oTileLayer;
        oTileLayer["type"]="Layer";
        writeLayerAttributes(oTileLayer, tileLayer);
        s.beginGroup("Storage");
        if (s.value("LayerDataFormat", 0).toInt()>0){
            oTileLayer["encoding"]="base64";
            QByteArray tileData;
            tileData.reserve(tileLayer->height() * tileLayer->width() * 4);
            for (int y = 0; y < tileLayer->height(); ++y) {
                for (int x = 0; x < tileLayer->width(); ++x) {
                    const uint gid = gidForCell(tileLayer->cellAt(x, y));
                    tileData.append((char) (gid));
                    tileData.append((char) (gid >> 8));
                    tileData.append((char) (gid >> 16));
                    tileData.append((char) (gid >> 24));
                }
            }
            oTileLayer["data"]=tileData.toBase64();
        }else{
            QVariantList oTiles;
            for (int y = 0; y < tileLayer->height(); ++y) {
                for (int x = 0; x < tileLayer->width(); ++x) {
                    oTiles << gidForCell(tileLayer->cellAt(x, y));
                }
            }
            oTileLayer["tiles"]=oTiles;
        }
        s.endGroup();
        return oTileLayer;
    }

    QVariantMap getObjectGroup(const ObjectGroup *objectGroup){
        QVariantMap oObjectGroup;
        oObjectGroup["type"]="ObjectGroup";
        if (objectGroup->color().isValid())
            oObjectGroup["color"]=objectGroup->color().name();

        writeLayerAttributes(oObjectGroup, objectGroup);
        QVariantList oObjects;
        foreach (const MapObject *mapObject, objectGroup->objects()){
            QVariantMap oObject;
            const QString &name = mapObject->name();
            const QString &type = mapObject->type();
            if (!name.isEmpty())
                oObject["name"]=name;
            if (!type.isEmpty())
                oObject["type"]=type;
            if (mapObject->tile())
                oObject["gid"]=gidForCell(Cell(mapObject->tile()));


            // Convert from tile to pixel coordinates
            const ObjectGroup *objectGroup = mapObject->objectGroup();
            const Map *map = objectGroup->map();
            const int tileHeight = map->tileHeight();
            const int tileWidth = map->tileWidth();
            const QRectF bounds = mapObject->bounds();

            int x, y, width, height;

            if (map->orientation() == Map::Isometric) {
                // Isometric needs special handling, since the pixel values are based
                // solely on the tile height.
                x = qRound(bounds.x() * tileHeight);
                y = qRound(bounds.y() * tileHeight);
                width = qRound(bounds.width() * tileHeight);
                height = qRound(bounds.height() * tileHeight);
            } else {
                x = qRound(bounds.x() * tileWidth);
                y = qRound(bounds.y() * tileHeight);
                width = qRound(bounds.width() * tileWidth);
                height = qRound(bounds.height() * tileHeight);
            }

            oObject["x"]=x;
            oObject["y"]=y;

            if (width != 0)
                oObject["width"]=width;
            if (height != 0)
                oObject["height"]=height;
            const Properties properties = mapObject->properties();
            if (!properties.isEmpty()) {
                oObject["properties"]=getProperties(properties);
            }
            oObjects << oObject;
        }

        oObjectGroup["objects"]=oObjects;
        return oObjectGroup;
    }

    void writeLayerAttributes(QVariantMap &oLayer, const Layer *layer)
    {
        oLayer["name"]=layer->name();
        oLayer["width"]=layer->width();
        oLayer["height"]=layer->height();

        const int x = layer->x();
        const int y = layer->y();
        const qreal opacity = layer->opacity();
        if (x != 0)
            oLayer["x"]=x;
        if (y != 0)
            oLayer["y"]=y;
        if (!layer->isVisible())
            oLayer["visible"]= "0";
        if (opacity != qreal(1))
            oLayer["opacity"]=opacity;

        const Properties properties = layer->properties();
        if (!properties.isEmpty()) {
            oLayer["properties"]=getProperties(properties);
        }
    }

    uint gidForCell(const Cell &cell) const
    {
        if (cell.isEmpty())
            return 0;

        const Tileset *tileset = cell.tile->tileset();

        // Find the first GID for the tileset
        QMap<uint, const Tileset*>::const_iterator i = mFirstGidToTileset.begin();
        QMap<uint, const Tileset*>::const_iterator i_end = mFirstGidToTileset.end();
        while (i != i_end && i.value() != tileset)
            ++i;

        if (i == i_end) // tileset not found
            return 0;

        uint gid = i.key() + cell.tile->id();
        if (cell.flippedHorizontally)
            gid |= FlippedHorizontallyFlag;
        if (cell.flippedVertically)
            gid |= FlippedVerticallyFlag;

        return gid;
    }

    QVariantMap variant(){
        return oMap;
    }

protected:
    const Map *tMap;
    QVariantMap oMap;
    QDir mapDir;
    QMap<uint, const Tileset*> mFirstGidToTileset;
};

QVariantMap mapToVariant(const Map *tMap, QDir mapDir){
    return MapToVariantHelper(tMap,mapDir).variant();
}

