/*
 * mapreader.h
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

#ifndef MAPREADER_H
#define MAPREADER_H

#include "tiled_global.h"

#include <QImage>

class QFile;

namespace Tiled {

class Map;
class Tileset;

namespace Internal {
class MapReaderPrivate;
}

/**
 * A fast QXmlStreamReader based reader for the TMX and TSX formats.
 *
 * Can be subclassed when special handling of external images and tilesets is
 * needed.
 */
class TILEDSHARED_EXPORT MapReader
{
public:
    MapReader();
    ~MapReader();

    /**
     * Reads a TMX map from the given \a device. Optionally a \a path can
     * be given, which will be used to resolve relative references to external
     * images and tilesets.
     *
     * Returns 0 and sets errorString() when reading failed.
     *
     * The caller takes ownership over the newly created tileset.
     */
    Map *readMap(QIODevice *device, const QString &path = QString());

    /**
     * Reads a TMX map from the given \a fileName.
     * \overload
     */
    Map *readMap(const QString &fileName);

    /**
     * Reads a TSX tileset from the given \a device. Optionally a \a path can
     * be given, which will be used to resolve relative references to external
     * images and tilesets.
     *
     * Returns 0 and sets errorString() when reading failed.
     *
     * The caller takes ownership over the newly created tileset.
     */
    Tileset *readTileset(QIODevice *device, const QString &path = QString());

    /**
     * Reads a TSX tileset from the given \a fileName.
     * \overload
     */
    Tileset *readTileset(const QString &fileName);

    /**
     * Returns the error message for the last occurred error.
     */
    QString errorString() const;

protected:
    /**
     * Called for each \a reference to an external file. Should return the path
     * to be used when loading this file. \a mapPath contains the path to the
     * map or tileset that is currently being loaded.
     */
    virtual QString resolveReference(const QString &reference,
                                     const QString &mapPath);

    /**
     * Called when an external image is encountered while a tileset is loaded.
     */
    virtual QImage readExternalImage(const QString &source);

    /**
     * Called when an external tileset is encountered while a map is loaded.
     * The default implementation just calls readTileset() on a new MapReader.
     *
     * If an error occurred, the \a error parameter should be set to the error
     * message.
     */
    virtual Tileset *readExternalTileset(const QString &source,
                                         QString *error);

private:
    friend class Internal::MapReaderPrivate;
    Internal::MapReaderPrivate *d;
};

} // namespace Tiled

#endif // MAPREADER_H
