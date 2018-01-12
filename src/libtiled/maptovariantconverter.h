/*
 * maptovariantconverter.h
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

#include <QDir>
#include <QVariant>

#include "gidmapper.h"

namespace Tiled {

class GroupLayer;
class ObjectTemplate;
struct TextData;

/**
 * Converts Map instances to QVariant. Meant to be used together with
 * JsonWriter.
 */
class TILEDSHARED_EXPORT MapToVariantConverter
{
public:
    MapToVariantConverter() {}

    /**
     * Converts the given \a map to a QVariant. The \a mapDir is used to
     * construct relative paths to external resources.
     */
    QVariant toVariant(const Map &map, const QDir &mapDir);

    /**
     * Converts the given \a tileset to a QVariant. The \a directory is used to
     * construct relative paths to external resources.
     */
    QVariant toVariant(const Tileset &tileset, const QDir &directory);
    QVariant toVariant(const ObjectTemplate &objectTemplate, const QDir &directory);

private:
    QVariant toVariant(const Tileset &tileset, int firstGid) const;
    QVariant toVariant(const Properties &properties) const;
    QVariant propertyTypesToVariant(const Properties &properties) const;
    QVariant toVariant(const QList<Layer*> &layers, Map::LayerDataFormat format) const;
    QVariant toVariant(const TileLayer &tileLayer, Map::LayerDataFormat format) const;
    QVariant toVariant(const ObjectGroup &objectGroup) const;
    QVariant toVariant(const MapObject &object) const;
    QVariant toVariant(const TextData &textData) const;
    QVariant toVariant(const ImageLayer &imageLayer) const;
    QVariant toVariant(const GroupLayer &groupLayer, Map::LayerDataFormat format) const;

    void addTileLayerData(QVariantMap &variant,
                          const TileLayer &tileLayer,
                          Map::LayerDataFormat format,
                          const QRect &bounds) const;

    void addLayerAttributes(QVariantMap &layerVariant,
                            const Layer &layer) const;

    void addProperties(QVariantMap &variantMap,
                       const Properties &properties) const;

    QDir mMapDir;
    GidMapper mGidMapper;
};

} // namespace Tiled
