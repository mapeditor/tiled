/*
 * varianttomapconverter.h
 * Copyright 2011, Porfírio José Pereira Ribeiro <porfirioribeiro@gmail.com>
 * Copyright 2011-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include "gidmapper.h"
#include "mapobject.h"

#include <QCoreApplication>
#include <QDir>
#include <QVariant>

namespace Tiled {

class GroupLayer;
class Layer;
class Map;
class ObjectGroup;
class ObjectTemplate;
class Properties;
class Tileset;

/**
 * Converts a QVariant to a Map instance. Meant to be used together with
 * JsonReader.
 */
class TILEDSHARED_EXPORT VariantToMapConverter
{
    // Using the MapReader context since the messages are the same
    Q_DECLARE_TR_FUNCTIONS(MapReader)

public:
    VariantToMapConverter()
        : mMap(nullptr)
        , mReadingExternalTileset(false)
    {}

    /**
     * Tries to convert the given \a variant to a Map instance. The \a mapDir
     * is necessary to resolve any relative references to external images.
     *
     * Returns 0 in case of an error. The error can be obstained using
     * errorString().
     */
    Map *toMap(const QVariant &variant, const QDir &mapDir);

    /**
     * Tries to convert the given \a variant to a Tileset instance. The
     * \a directory is necessary to resolve any relative references to external
     * images.
     *
     * Returns 0 in case of an error. The error can be obstained using
     * errorString().
     */
    SharedTileset toTileset(const QVariant &variant, const QDir &directory);

    /**
     * Tries to convert the given \a variant to an ObjectTemplate instance. The
     * \a directory is necessary to resolve any relative references to external
     * tilesets.
     */
    ObjectTemplate *toObjectTemplate(const QVariant &variant, const QDir &directory);

    /**
     * Returns the last error, if any.
     */
    QString errorString() const { return mError; }

private:
    Properties toProperties(const QVariant &propertiesVariant,
                            const QVariant &propertyTypesVariant) const;
    SharedTileset toTileset(const QVariant &variant);
    ObjectTemplate *toObjectTemplate(const QVariant &variant);
    Layer *toLayer(const QVariant &variant);
    TileLayer *toTileLayer(const QVariantMap &variantMap);
    ObjectGroup *toObjectGroup(const QVariantMap &variantMap);
    MapObject *toMapObject(const QVariantMap &variantMap);
    ImageLayer *toImageLayer(const QVariantMap &variantMap);
    GroupLayer *toGroupLayer(const QVariantMap &variantMap);

    QPolygonF toPolygon(const QVariant &variant) const;
    TextData toTextData(const QVariantMap &variant) const;

    Properties extractProperties(const QVariantMap &variantMap) const;

    Map *mMap;
    QDir mMapDir;
    bool mReadingExternalTileset;
    GidMapper mGidMapper;
    QString mError;
};

} // namespace Tiled
