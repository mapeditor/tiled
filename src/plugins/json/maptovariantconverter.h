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

#ifndef MAPTOVARIANTCONVERTER_H
#define MAPTOVARIANTCONVERTER_H

#include <QDir>
#include <QVariant>

#include "gidmapper.h"

namespace Json {

/**
 * Converts Map instances to QVariant. Meant to be used together with
 * JsonWriter.
 */
class MapToVariantConverter
{
public:
    MapToVariantConverter() {}

    /**
     * Converts the given \s map to a QVariant. The \a mapDir is used to
     * construct relative paths to external resources.
     */
    QVariant toVariant(const Tiled::Map *map, const QDir &mapDir);

private:
    QVariant toVariant(const Tiled::Tileset *tileset, int firstGid) const;
    QVariant toVariant(const Tiled::Properties &properties) const;
    QVariant toVariant(const Tiled::TileLayer *tileLayer) const;
    QVariant toVariant(const Tiled::ObjectGroup *objectGroup) const;
    QVariant toVariant(const Tiled::ImageLayer *imageLayer) const;

    void addLayerAttributes(QVariantMap &layerVariant,
                            const Tiled::Layer *layer) const;

    QDir mMapDir;
    Tiled::GidMapper mGidMapper;
};

} // namespace Json

#endif // MAPTOVARIANTCONVERTER_H
