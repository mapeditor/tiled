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

#include "compression.h"

#include <zlib.h>
#include <QByteArray>
#include <QDebug>

using namespace Tiled::Internal;

// TODO: Improve error reporting by showing these errors in the user interface
static void logZlibError(int error)
{
    switch (error)
    {
        case Z_MEM_ERROR:
            qDebug() << "Out of memory while (de)compressing data!";
            break;
        case Z_VERSION_ERROR:
            qDebug() << "Incompatible zlib version!";
            break;
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
            qDebug() << "Incorrect zlib compressed data!";
            break;
        default:
            qDebug() << "Unknown error while (de)compressing data!";
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
    strm.next_out = (Bytef *) out;
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

QByteArray Tiled::Internal::compress(const QByteArray &data,
                                     CompressionMethod method)
{
    int bufferSize = 1024;
    char *out = (char *) malloc(bufferSize);
    int err;
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = (Bytef *) data.data();
    strm.avail_in = data.length();
    strm.next_out = (Bytef *) out;
    strm.avail_out = bufferSize;

    const int windowBits = (method == Gzip) ? 15 + 16 : 15;

    err = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits,
                       8, Z_DEFAULT_STRATEGY);
    if (err != Z_OK) {
        logZlibError(err);
        return QByteArray();
    }

    do {
        err = deflate(&strm, Z_FINISH);
        Q_ASSERT(err != Z_STREAM_ERROR);

        if (err == Z_OK) {
            // More output space needed
            out = (char *) realloc(out, bufferSize * 2);
            strm.next_out = (Bytef *)(out + bufferSize);
            strm.avail_out = bufferSize;
            bufferSize *= 2;
        }
    } while (err == Z_OK);

    if (err != Z_STREAM_END) {
        logZlibError(err);
        deflateEnd(&strm);
        return QByteArray();
    }

    const int outLength = bufferSize - strm.avail_out;
    deflateEnd(&strm);

    QByteArray outByteArray(out, outLength);
    free(out);
    return outByteArray;
}
