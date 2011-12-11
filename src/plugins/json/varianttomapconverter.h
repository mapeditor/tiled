/*
 * JSON Tiled Plugin
 * Copyright 2011, Porfírio José Pereira Ribeiro <porfirioribeiro@gmail.com>
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef VARIANTTOMAPCONVERTER_H
#define VARIANTTOMAPCONVERTER_H

#include "gidmapper.h"

#include <QCoreApplication>
#include <QDir>
#include <QVariant>

namespace Tiled {
class Layer;
class Map;
class ObjectGroup;
class Properties;
class Tileset;
}

namespace Json {

/**
 * Converts a QVariant to a Map instance. Meant to be used together with
 * JsonReader.
 */
class VariantToMapConverter
{
    // Using the MapReader context since the messages are the same
    Q_DECLARE_TR_FUNCTIONS(MapReader)

public:
    VariantToMapConverter() {}

    /**
     * Tries to convert the given \a variant to a Map instance. The \a mapDir
     * is necessary to resolve any relative references to external images.
     *
     * Returns 0 in case of an error. The error can be obstained using
     * errorString().
     */
    Tiled::Map *toMap(const QVariant &variant, const QDir &mapDir);

    /**
     * Returns the last error, if any.
     */
    QString errorString() const { return mError; }

private:
    Tiled::Properties toProperties(const QVariant &variant);
    Tiled::Tileset *toTileset(const QVariant &variant);
    Tiled::Layer *toLayer(const QVariant &variant);
    Tiled::TileLayer *toTileLayer(const QVariantMap &variantMap);
    Tiled::ObjectGroup *toObjectGroup(const QVariantMap &variantMap);

    QPolygonF toPolygon(const QVariant &variant) const;

    Tiled::Map *mMap;
    QDir mMapDir;
    Tiled::GidMapper mGidMapper;
    QString mError;
};

} // namespace Json

#endif // VARIANTTOMAPCONVERTER_H
