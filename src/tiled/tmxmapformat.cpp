/*
 * tmxmapformat.cpp
 * Copyright 2008-2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tmxmapformat.h"

#include "map.h"
#include "mapreader.h"
#include "mapwriter.h"
#include "preferences.h"
#include "tilesetmanager.h"

#include <QBuffer>
#include <QDir>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

class EditorMapReader : public MapReader
{
protected:
    /**
     * Overridden to make sure the resolved reference is a clean path.
     */
    QString resolveReference(const QString &reference, const QString &mapPath)
    {
        QString resolved = MapReader::resolveReference(reference, mapPath);
        return QDir::cleanPath(resolved);
    }

    /**
     * Overridden in order to check with the TilesetManager whether the tileset
     * is already loaded.
     */
    SharedTileset readExternalTileset(const QString &source, QString *error) override
    {
        // Check if this tileset is already loaded
        TilesetManager *manager = TilesetManager::instance();
        SharedTileset tileset = manager->findTileset(source);

        // If not, try to load it
        if (!tileset)
            tileset = MapReader::readExternalTileset(source, error);

        return tileset;
    }
};

} // anonymous namespace

Map *TmxMapFormat::read(const QString &fileName)
{
    mError.clear();

    EditorMapReader reader;
    Map *map = reader.readMap(fileName);
    if (!map)
        mError = reader.errorString();

    return map;
}

SharedTileset TmxMapFormat::readTileset(const QString &fileName)
{
    mError.clear();

    EditorMapReader reader;
    SharedTileset tileset = reader.readTileset(fileName);
    if (!tileset)
        mError = reader.errorString();

    return tileset;
}

bool TmxMapFormat::write(const Map *map, const QString &fileName)
{
    Preferences *prefs = Preferences::instance();

    MapWriter writer;
    writer.setDtdEnabled(prefs->dtdEnabled());

    bool result = writer.writeMap(map, fileName);
    if (!result)
        mError = writer.errorString();
    else
        mError.clear();

    return result;
}

bool TmxMapFormat::writeTileset(const Tileset &tileset,
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

QByteArray TmxMapFormat::toByteArray(const Map *map)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    MapWriter writer;
    writer.writeMap(map, &buffer);

    return buffer.data();
}

Map *TmxMapFormat::fromByteArray(const QByteArray &data)
{
    mError.clear();

    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QBuffer::ReadOnly);

    EditorMapReader reader;
    Map *map = reader.readMap(&buffer);
    if (!map)
        mError = reader.errorString();

    return map;
}
