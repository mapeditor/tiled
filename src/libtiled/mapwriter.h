/*
 * mapwriter.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Dennis Honeyman <arcticuno@gmail.com>
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

#pragma once

#include "map.h"
#include "tiled_global.h"

#include <QString>

class QIODevice;

namespace Tiled {

class Map;
class MapObject;
class ObjectTemplate;
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
    void writeTileset(const Tileset &tileset, QIODevice *device,
                      const QString &path = QString());

    /**
     * Writes a TSX tileset to the given \a fileName.
     *
     * Returns false and sets errorString() when reading failed.
     * \overload
     */
    bool writeTileset(const Tileset &tileset, const QString &fileName);

    void writeObjectTemplate(const ObjectTemplate *objectTemplate, QIODevice *device,
                             const QString &path = QString());

    bool writeObjectTemplate(const ObjectTemplate *objectTemplate, const QString &fileName);

    /**
     * Returns the error message for the last occurred error.
     */
    QString errorString() const;

    /**
     * Sets whether the DTD reference is written when saving the map.
     */
    void setDtdEnabled(bool enabled);
    bool isDtdEnabled() const;

private:
    Q_DISABLE_COPY(MapWriter)

    Internal::MapWriterPrivate *d;
};

} // namespace Tiled
