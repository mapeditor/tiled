/*
 * compression.cpp
 * Copyright 2008, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "compression.h"

#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
#include "QtZlib/zlib.h"
#else
#include <zlib.h>
#endif
#ifdef TILED_ZSTD_SUPPORT
#include <zstd.h>      // presumes zstd library is installed
#endif

#include <QByteArray>
#include <QDebug>

#include "qtcompat_p.h"

#ifdef Z_PREFIX
#undef compress
#endif

using namespace Tiled;

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

QByteArray Tiled::decompress(const QByteArray &data,
                             int expectedSize,
                             CompressionMethod method)
{
    if (data.isEmpty())
        return QByteArray();

    QByteArray out;
    out.resize(expectedSize);
    if (method == Zlib || method == Gzip) {
        z_stream strm;

        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = (Bytef *) data.data();
        strm.avail_in = data.length();
        strm.next_out = (Bytef *) out.data();
        strm.avail_out = out.size();

        int ret = inflateInit2(&strm, 15 + 32);

        if (ret != Z_OK) {
            logZlibError(ret);
            return QByteArray();
        }

        do {
            ret = inflate(&strm, Z_SYNC_FLUSH);
            Q_ASSERT(ret != Z_STREAM_ERROR);

            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                    Q_FALLTHROUGH();
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&strm);
                    logZlibError(ret);
                    return QByteArray();
            }

            if (ret != Z_STREAM_END) {
                int oldSize = out.size();
                out.resize(oldSize * 2);

                strm.next_out = (Bytef *)(out.data() + oldSize);
                strm.avail_out = oldSize;
            }
        }
        while (ret != Z_STREAM_END);

        if (strm.avail_in != 0) {
            logZlibError(Z_DATA_ERROR);
            return QByteArray();
        }

        const int outLength = out.size() - strm.avail_out;
        inflateEnd(&strm);

        out.resize(outLength);
        return out;
#ifdef TILED_ZSTD_SUPPORT
    } else if (method == Zstandard) {
        size_t const dSize = ZSTD_decompress(out.data(), out.size(), data.constData(), data.size());
        if (ZSTD_isError(dSize)) {
            qDebug() << "error decoding:" << ZSTD_getErrorName(dSize);
            return QByteArray();
        }
        out.resize(dSize);
        return out;
#endif
    } else {
        qDebug() << "compression not supported:" << method;
        return QByteArray();
    }
}

QByteArray Tiled::compress(const QByteArray &data,
                           CompressionMethod method,
                           int compressionLevel)
{
    if (data.isEmpty())
        return QByteArray();

    if (method == Zlib || method == Gzip) {
        if (compressionLevel == -1)
            compressionLevel = Z_DEFAULT_COMPRESSION;
        else
            compressionLevel = qBound(Z_BEST_SPEED, compressionLevel, Z_BEST_COMPRESSION);

        QByteArray out;
        out.resize(1024);
        int err;
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.next_in = (Bytef *) data.data();
        strm.avail_in = data.length();
        strm.next_out = (Bytef *) out.data();
        strm.avail_out = out.size();

        const int windowBits = (method == Gzip) ? 15 + 16 : 15;

        err = deflateInit2(&strm, compressionLevel, Z_DEFLATED, windowBits,
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
                int oldSize = out.size();
                out.resize(out.size() * 2);

                strm.next_out = (Bytef *)(out.data() + oldSize);
                strm.avail_out = oldSize;
            }
        } while (err == Z_OK);

        if (err != Z_STREAM_END) {
            logZlibError(err);
            deflateEnd(&strm);
            return QByteArray();
        }

        const int outLength = out.size() - strm.avail_out;
        deflateEnd(&strm);

        out.resize(outLength);
        return out;
#ifdef TILED_ZSTD_SUPPORT
    } else if (method == Zstandard) {
        if (compressionLevel == -1)
            compressionLevel = 6;
        else
            compressionLevel = qBound(1, compressionLevel, 22);

        size_t const cBuffSize = ZSTD_compressBound(data.size());

        QByteArray out;
        out.resize(cBuffSize);

        size_t const cSize = ZSTD_compress(out.data(), cBuffSize, data.constData(), data.size(), compressionLevel);
        if (ZSTD_isError(cSize)) {
            qDebug() << "error compressing:" << ZSTD_getErrorName(cSize);
            return QByteArray();
        }

        out.resize(cSize);
        return out;
#endif
    } else {
        qDebug() << "compression not supported:" << method;
        return QByteArray();
    }
}
