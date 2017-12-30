/*
 * gidmapper.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "gidmapper.h"

#include "compression.h"
#include "tile.h"
#include "tiled.h"
#include "tileset.h"

using namespace Tiled;

// Bits on the far end of the 32-bit global tile ID are used for tile flags
const unsigned FlippedHorizontallyFlag   = 0x80000000;
const unsigned FlippedVerticallyFlag     = 0x40000000;
const unsigned FlippedAntiDiagonallyFlag = 0x20000000;

const unsigned RotatedHexagonal120Flag   = 0x10000000;

/**
 * Default constructor. Use \l insert to initialize the gid mapper
 * incrementally.
 */
GidMapper::GidMapper()
    : mInvalidTile(0)
{
}

/**
 * Constructor that initializes the gid mapper using the given \a tilesets.
 */
GidMapper::GidMapper(const QVector<SharedTileset> &tilesets)
    : GidMapper()
{
    unsigned firstGid = 1;
    for (const SharedTileset &tileset : tilesets) {
        insert(firstGid, tileset);
        firstGid += tileset->nextTileId();
    }
}

/**
 * Returns the cell data matched by the given \a gid. The \a ok parameter
 * indicates whether an error occurred.
 */
Cell GidMapper::gidToCell(unsigned gid, bool &ok) const
{
    Cell result;

    // Read out the flags
    result.setFlippedHorizontally(gid & FlippedHorizontallyFlag);
    result.setFlippedVertically(gid & FlippedVerticallyFlag);
    result.setFlippedAntiDiagonally(gid & FlippedAntiDiagonallyFlag);

    result.setRotatedHexagonal120(gid & RotatedHexagonal120Flag);

    // Clear the flags
    gid &= ~(FlippedHorizontallyFlag |
             FlippedVerticallyFlag |
             FlippedAntiDiagonallyFlag |
             RotatedHexagonal120Flag);

    if (gid == 0) {
        ok = true;
    } else if (isEmpty()) {
        ok = false;
    } else {
        // Find the tileset containing this tile
        QMap<unsigned, SharedTileset>::const_iterator i = mFirstGidToTileset.upperBound(gid);
        if (i == mFirstGidToTileset.begin()) {
            // Invalid global tile ID, since it lies before the first tileset
            ok = false;
        } else {
            --i; // Navigate one tileset back since upper bound finds the next
            int tileId = gid - i.key();
            const SharedTileset &tileset = i.value();

            result.setTile(tileset.data(), tileId);

            ok = true;
        }
    }

    return result;
}

/**
 * Returns the global tile ID for the given \a cell. Returns 0 when the cell is
 * empty or when its tileset isn't known.
 */
unsigned GidMapper::cellToGid(const Cell &cell) const
{
    if (cell.isEmpty())
        return 0;

    const Tileset *tileset = cell.tileset();

    // Find the first GID for the tileset
    QMap<unsigned, SharedTileset>::const_iterator i = mFirstGidToTileset.begin();
    QMap<unsigned, SharedTileset>::const_iterator i_end = mFirstGidToTileset.end();
    while (i != i_end && i.value() != tileset)
        ++i;

    if (i == i_end) // tileset not found
        return 0;

    unsigned gid = i.key() + cell.tileId();
    if (cell.flippedHorizontally())
        gid |= FlippedHorizontallyFlag;
    if (cell.flippedVertically())
        gid |= FlippedVerticallyFlag;
    if (cell.flippedAntiDiagonally())
        gid |= FlippedAntiDiagonallyFlag;
    if (cell.rotatedHexagonal120())
        gid |= RotatedHexagonal120Flag;

    return gid;
}

/**
 * Encodes the tile layer data of the given \a tileLayer in the given
 * \a format. This function should only be used for base64 encoding, with or
 * without compression.
 */
QByteArray GidMapper::encodeLayerData(const TileLayer &tileLayer,
                                      Map::LayerDataFormat format,
                                      QRect bounds) const
{
    Q_ASSERT(format != Map::XML);
    Q_ASSERT(format != Map::CSV);

    if (bounds.isEmpty())
        bounds = QRect(0, 0, tileLayer.width(), tileLayer.height());

    QByteArray tileData;
    tileData.reserve(bounds.width() * bounds.height() * 4);

    for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
        for (int x = bounds.left(); x <= bounds.right(); ++x) {
            const unsigned gid = cellToGid(tileLayer.cellAt(x, y));
            tileData.append(static_cast<char>(gid));
            tileData.append(static_cast<char>(gid >> 8));
            tileData.append(static_cast<char>(gid >> 16));
            tileData.append(static_cast<char>(gid >> 24));
        }
    }

    if (format == Map::Base64Gzip)
        tileData = compress(tileData, Gzip);
    else if (format == Map::Base64Zlib)
        tileData = compress(tileData, Zlib);

    return tileData.toBase64();
}

GidMapper::DecodeError GidMapper::decodeLayerData(TileLayer &tileLayer,
                                                  const QByteArray &layerData,
                                                  Map::LayerDataFormat format,
                                                  QRect bounds) const
{
    Q_ASSERT(format != Map::XML);
    Q_ASSERT(format != Map::CSV);

    if (bounds.isEmpty())
        bounds = QRect(0, 0, tileLayer.width(), tileLayer.height());

    QByteArray decodedData = QByteArray::fromBase64(layerData);
    const int size = (bounds.width() * bounds.height()) * 4;

    if (format == Map::Base64Gzip || format == Map::Base64Zlib)
        decodedData = decompress(decodedData, size);

    if (size != decodedData.length())
        return CorruptLayerData;

    const unsigned char *data = reinterpret_cast<const unsigned char*>(decodedData.constData());
    int x = bounds.x();
    int y = bounds.y();
    bool ok;

    for (int i = 0; i < size - 3; i += 4) {
        const unsigned gid = data[i] |
                             data[i + 1] << 8 |
                             data[i + 2] << 16 |
                             data[i + 3] << 24;

        const Cell result = gidToCell(gid, ok);
        if (!ok) {
            mInvalidTile = gid;
            return isEmpty() ? TileButNoTilesets : InvalidTile;
        }

        tileLayer.setCell(x, y, result);

        x++;
        if (x == bounds.right() + 1) {
            x = bounds.x();
            y++;
        }
    }

    return NoError;
}
