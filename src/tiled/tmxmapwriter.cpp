/*
 * tmxmapwriter.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tmxmapwriter.h"

#include "mapwriter.h"
#include "preferences.h"

#include <QBuffer>

using namespace Tiled;
using namespace Tiled::Internal;

bool TmxMapWriter::write(const Map *map, const QString &fileName)
{
    Preferences *prefs = Preferences::instance();

    MapWriter writer;
    writer.setLayerDataFormat(prefs->layerDataFormat());
    writer.setDtdEnabled(prefs->dtdEnabled());

    bool result = writer.writeMap(map, fileName);
    if (!result)
        mError = writer.errorString();
    else
        mError.clear();

    return result;
}

bool TmxMapWriter::writeTileset(const Tileset *tileset,
                                const QString &fileName)
{
    Preferences *prefs = Preferences::instance();

    MapWriter writer;
    writer.setDtdEnabled(prefs->dtdEnabled());

    bool result = writer.writeTileset(tileset, fileName);
    if (!result)
        mError = writer.errorString();
    else
        mError.clear();

    return result;
}

QByteArray TmxMapWriter::toByteArray(const Map *map)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);

    MapWriter writer;
    writer.setLayerDataFormat(MapWriter::Base64Zlib);
    writer.writeMap(map, &buffer);

    return bytes;
}
