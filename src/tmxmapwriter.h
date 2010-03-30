/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef TMXMAPWRITER_H
#define TMXMAPWRITER_H

#include "mapwriterinterface.h"

#include <QCoreApplication>
#include <QDir>
#include <QMap>
#include <QString>
#include <QXmlStreamWriter>

namespace Tiled {

class Layer;
class MapObject;
class ObjectGroup;
class Tile;
class TileLayer;
class Tileset;

namespace Internal {

/**
 * A writer for Tiled's .tmx map format.
 */
class TmxMapWriter : public MapWriterInterface
{
    Q_DECLARE_TR_FUNCTIONS(TmxMapReader)

public:
    TmxMapWriter();

    bool write(const Map *map, const QString &fileName);
    
    /**
     * Converts the given map to a string (in .tmx format). This is for
     * storing a map in the clipboard. References to other files (like tileset
     * images) will be saved as absolute paths.
     *
     * @see TmxMapReader::fromString
     */
    QString toString(const Map *map);

    QString nameFilter() const { return tr("Tiled map files (*.tmx)"); }

    QString errorString() const { return mError; }

    /**
     * The different formats in which the tile layer data can be stored.
     */
    enum LayerDataFormat {
        XML        = 0,
        Base64     = 1,
        Base64Gzip = 2,
        Base64Zlib = 3,
        CSV        = 4,
    };

    /**
     * Sets the format in which the tile layer data is stored.
     */
    void setLayerDataFormat(LayerDataFormat format)
    { mLayerDataFormat = format; }

    LayerDataFormat layerDataFormat() const
    { return mLayerDataFormat; }

    /**
     * Sets whether the DTD reference is written when saving the map.
     */
    void setDtdEnabled(bool enabled)
    { mDtdEnabled = enabled; }

    bool isDtdEnabled() const
    { return mDtdEnabled; }

private:
    void writeMap(QXmlStreamWriter &w, const Map *map);
    void writeTileset(QXmlStreamWriter &w, const Tileset *tileset,
                      int firstGid);
    void writeTileLayer(QXmlStreamWriter &w, const TileLayer *tileLayer);
    void writeLayerAttributes(QXmlStreamWriter &w, const Layer *layer);
    int gidForTile(const Tile *tile) const;
    void writeObjectGroup(QXmlStreamWriter &w, const ObjectGroup *objectGroup);
    void writeObject(QXmlStreamWriter &w, const MapObject *mapObject);
    void writeProperties(QXmlStreamWriter &w,
                         const QMap<QString, QString> &properties);

    QString mError;
    QDir mMapDir;     // The directory in which the map is being saved
    QMap<int, const Tileset*> mFirstGidToTileset;
    bool mUseAbsolutePaths;
    LayerDataFormat mLayerDataFormat;
    bool mDtdEnabled;
};

} // namespace Internal
} // namespace Tiled

#endif // TMXMAPWRITER_H
