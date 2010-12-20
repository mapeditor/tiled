/*
 * tmxmapreader.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Dennis Honeyman <arcticuno@gmail.com>
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

#include "tmxmapreader.h"

#include "map.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "mapreader.h"

#include <QBuffer>
#include <QFileInfo>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

class EditorMapReader : public MapReader
{
protected:
    /**
     * Overridden to make sure the resolved reference is canonical.
     */
    QString resolveReference(const QString &reference, const QString &mapPath)
    {
        QString resolved = MapReader::resolveReference(reference, mapPath);
        QString canonical = QFileInfo(resolved).canonicalFilePath();

        // Make sure that we're not returning an empty string when the file is
        // not found.
        return canonical.isEmpty() ? resolved : canonical;
    }

    /**
     * Overridden in order to check with the TilesetManager whether the tileset
     * is already loaded.
     */
    Tileset *readExternalTileset(const QString &source, QString *error)
    {
        // Check if this tileset is already loaded
        TilesetManager *manager = TilesetManager::instance();
        Tileset *tileset = manager->findTileset(source);

        // If not, try to load it
        if (!tileset)
            tileset = MapReader::readExternalTileset(source, error);

        return tileset;
    }
};

} // anonymous namespace

Map *TmxMapReader::read(const QString &fileName)
{
    mError.clear();

    EditorMapReader reader;
    Map *map = reader.readMap(fileName);
    if (!map)
        mError = reader.errorString();

    return map;
}

Map *TmxMapReader::fromByteArray(const QByteArray &data)
{
    mError.clear();

    QByteArray dataCopy = data;
    QBuffer buffer(&dataCopy);
    buffer.open(QBuffer::ReadOnly);

    EditorMapReader reader;
    Map *map = reader.readMap(&buffer);
    if (!map)
        mError = reader.errorString();

    return map;
}

Tileset *TmxMapReader::readTileset(const QString &fileName)
{
    mError.clear();

    EditorMapReader reader;
    Tileset *tileset = reader.readTileset(fileName);
    if (!tileset)
        mError = reader.errorString();

    return tileset;
}
