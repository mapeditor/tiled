/*
 * compression.h
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

#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "tiled_global.h"

class QByteArray;

namespace Tiled {

enum CompressionMethod {
    Gzip,
    Zlib
};

/**
 * Decompresses either zlib or gzip compressed memory. Returns a null
 * QByteArray if decompressing failed.
 *
 * Needed because qUncompress does not support gzip compressed data. Also,
 * this method does not need the expected size to be prepended to the data,
 * but it can be passed as optional parameter.
 *
 * @param data         the compressed data
 * @param expectedSize the expected size of the uncompressed data in bytes
 * @return the uncompressed data, or a null QByteArray if decompressing failed
 */
QByteArray TILEDSHARED_EXPORT decompress(const QByteArray &data,
                                         int expectedSize = 1024);

/**
 * Compresses the give data in either gzip or zlib format. Returns a null
 * QByteArray if compression failed.
 *
 * Needed because qCompress does not support gzip compression.
 *
 * @param data the uncompressed data
 * @return the compressed data, or a null QByteArray if compression failed
 */
QByteArray TILEDSHARED_EXPORT compress(const QByteArray &data,
                                       CompressionMethod method = Zlib);

} // namespace Tiled

#endif // COMPRESSION_H
