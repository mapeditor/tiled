/*
 * mapwriter.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef MAPWRITER_H
#define MAPWRITER_H

#include "tiled_global.h"

#include <QString>

class QIODevice;

namespace Tiled {

class Map;
class Tileset;

namespace Internal {
class MapWriterPrivate;
}

/**
 * A QXmlStreamWriter based writer for the TMX and TSX formats.
 */
class TILEDSHARED_EXPORT MapWriter
{
public:
    MapWriter();
    ~MapWriter();

    /**
     * Writes a TMX map to the given \a device. Optionally a \a path can
     * be given, which will be used to create relative references to external
     * images and tilesets.
     *
     * Error checking will need to be done on the \a device after calling this
     * function.
     */
    void writeMap(const Map *map, QIODevice *device,
                  const QString &path = QString());

    /**
     * Writes a TMX map to the given \a fileName.
     *
     * Returns false and sets errorString() when reading failed.
     * \overload
     */
    bool writeMap(const Map *map, const QString &fileName);

    /**
     * Writes a TSX tileset to the given \a device. Optionally a \a path can
     * be given, which will be used to create relative references to external
     * images.
     *
     * Error checking will need to be done on the \a device after calling this
     * function.
     */
    void writeTileset(const Tileset *tileset, QIODevice *device,
                      const QString &path = QString());

    /**
     * Writes a TSX tileset to the given \a fileName.
     *
     * Returns false and sets errorString() when reading failed.
     * \overload
     */
    bool writeTileset(const Tileset *tileset, const QString &fileName);

    /**
     * Returns the error message for the last occurred error.
     */
    QString errorString() const;

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
    void setLayerDataFormat(LayerDataFormat format);
    LayerDataFormat layerDataFormat() const;

    /**
     * Sets whether the DTD reference is written when saving the map.
     */
    void setDtdEnabled(bool enabled);
    bool isDtdEnabled() const;

private:
    Internal::MapWriterPrivate *d;
};

} // namespace Tiled

#endif // MAPWRITER_H
