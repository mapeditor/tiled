/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "decompress.h"

#include <zlib.h>
#include <QByteArray>
#include <QDebug>

using namespace Tiled::Internal;

static void logZlibError(int error)
{
    switch (error)
    {
        case Z_MEM_ERROR:
            qDebug() << "Out of memory while decompressing data!";
            break;
        case Z_VERSION_ERROR:
            qDebug() << "Incompatible zlib version!";
            break;
        case Z_DATA_ERROR:
            qDebug() << "Incorrect zlib compressed data!";
            break;
        default:
            qDebug() << "Unknown error while decompressing data!";
    }
}

QByteArray Tiled::Internal::decompress(const QByteArray &data, int expectedSize)
{
    int bufferSize = expectedSize;
    int ret;
    z_stream strm;
    char *out = (char *) malloc(bufferSize);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = (Bytef *) data.data();
    strm.avail_in = data.length();
    strm.next_out = (Bytef *)out;
    strm.avail_out = bufferSize;

    ret = inflateInit2(&strm, 15 + 32);

    if (ret != Z_OK) {
        logZlibError(ret);
        free(out);
        return QByteArray();
    }

    do {
        ret = inflate(&strm, Z_SYNC_FLUSH);

        switch (ret) {
            case Z_NEED_DICT:
            case Z_STREAM_ERROR:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                logZlibError(ret);
                free(out);
                return QByteArray();
        }

        if (ret != Z_STREAM_END) {
            out = (char *) realloc(out, bufferSize * 2);

            if (!out) {
                inflateEnd(&strm);
                logZlibError(Z_MEM_ERROR);
                free(out);
                return QByteArray();
            }

            strm.next_out = (Bytef *)(out + bufferSize);
            strm.avail_out = bufferSize;
            bufferSize *= 2;
        }
    }
    while (ret != Z_STREAM_END);

    if (strm.avail_in != 0) {
        logZlibError(Z_DATA_ERROR);
        free(out);
        return QByteArray();
    }

    const int outLength = bufferSize - strm.avail_out;
    inflateEnd(&strm);

    QByteArray outByteArray(out, outLength);
    free(out);
    return outByteArray;
}
