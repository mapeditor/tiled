/*
 * mapreader.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
     * The caller takes ownership over the newly created map.
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
     * images.
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
