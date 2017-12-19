/*
 * mapformat.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "mapformat.h"

#include "mapreader.h"

namespace Tiled {

Map *readMap(const QString &fileName, QString *error)
{
    // Try the first registered map format that claims to support the file
    if (MapFormat *format = findSupportingMapFormat(fileName)) {
        Map *map = format->read(fileName);

        if (error) {
            if (!map)
                *error = format->errorString();
            else
                *error = QString();
        }

        return map;
    }

    // Fall back to default reader (TMX format)
    MapReader reader;
    Map *map = reader.readMap(fileName);

    if (error) {
        if (!map)
            *error = reader.errorString();
        else
            *error = QString();
    }

    return map;
}

MapFormat *findSupportingMapFormat(const QString &fileName)
{
    for (MapFormat *format : PluginManager::objects<MapFormat>())
        if (format->supportsFile(fileName))
            return format;
    return nullptr;
}

} // namespace Tiled
