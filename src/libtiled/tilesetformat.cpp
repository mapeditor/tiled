/*
 * tilesetformat.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilesetformat.h"

#include "mapreader.h"

namespace Tiled {

SharedTileset readTileset(const QString &fileName, QString *error)
{
    // Try the first registered tileset format that claims to support the file
    for (TilesetFormat *format : PluginManager::objects<TilesetFormat>()) {
        if (format->supportsFile(fileName)) {
            SharedTileset tileset = format->read(fileName);

            if (error) {
                if (!tileset)
                    *error = format->errorString();
                else
                    *error = QString();
            }

            return tileset;
        }
    }


    // Fall back to default reader (TSX format)
    MapReader reader;
    SharedTileset tileset = reader.readTileset(fileName);

    if (error) {
        if (!tileset)
            *error = reader.errorString();
        else
            *error = QString();
    }

    return tileset;
}

} // namespace Tiled
