/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2021 Albert Astals Cid <aacid@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kzstdfilter.h"
#include "loggingcategory.h"

#include <QIODevice>

#if HAVE_ZSTD_SUPPORT

extern "C" {
#include <zstd.h>
}

class Q_DECL_HIDDEN KZstdFilter::Private
{
public:
    union {
        ZSTD_CStream *cStream;
        ZSTD_DStream *dStream;
    };
    int mode;
    bool isInitialized = false;
    ZSTD_inBuffer inBuffer;
    ZSTD_outBuffer outBuffer;
};

KZstdFilter::KZstdFilter()
    : d(new Private)
{
}

KZstdFilter::~KZstdFilter()
{
}

bool KZstdFilter::init(int mode)
{
    if (d->isInitialized) {
        terminate();
    }

    d->inBuffer.size = 0;
    d->inBuffer.pos = 0;

    if (mode == QIODevice::ReadOnly) {
        d->dStream = ZSTD_createDStream();
    } else if (mode == QIODevice::WriteOnly) {
        d->cStream = ZSTD_createCStream();
    } else {
        // qCWarning(KArchiveLog) << "Unsupported mode " << mode << ". Only QIODevice::ReadOnly and QIODevice::WriteOnly supported";
        return false;
    }
    d->mode = mode;
    d->isInitialized = true;
    return true;
}

int KZstdFilter::mode() const
{
    return d->mode;
}

bool KZstdFilter::terminate()
{
    if (d->mode == QIODevice::ReadOnly) {
        ZSTD_freeDStream(d->dStream);
    } else if (d->mode == QIODevice::WriteOnly) {
        ZSTD_freeCStream(d->cStream);
    } else {
        // qCWarning(KArchiveLog) << "Unsupported mode " << d->mode << ". Only QIODevice::ReadOnly and QIODevice::WriteOnly supported";
        return false;
    }
    d->isInitialized = false;
    return true;
}

void KZstdFilter::reset()
{
    terminate();
    init(d->mode);
}

void KZstdFilter::setOutBuffer(char *data, uint maxlen)
{
    d->outBuffer.dst = data;
    d->outBuffer.size = maxlen;
    d->outBuffer.pos = 0;
}

void KZstdFilter::setInBuffer(const char *data, unsigned int size)
{
    d->inBuffer.src = data;
    d->inBuffer.size = size;
    d->inBuffer.pos = 0;
}

int KZstdFilter::inBufferAvailable() const
{
    return d->inBuffer.size - d->inBuffer.pos;
}

int KZstdFilter::outBufferAvailable() const
{
    return d->outBuffer.size - d->outBuffer.pos;
}

KZstdFilter::Result KZstdFilter::uncompress()
{
    // qCDebug(KArchiveLog) << "Calling ZSTD_decompressStream with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
    const size_t result = ZSTD_decompressStream(d->dStream, &d->outBuffer, &d->inBuffer);
    if (ZSTD_isError(result)) {
        qCWarning(KArchiveLog) << "ZSTD_decompressStream returned" << result << ZSTD_getErrorName(result);
        return KFilterBase::Error;
    }

    return result == 0 ? KFilterBase::End : KFilterBase::Ok;
}

KZstdFilter::Result KZstdFilter::compress(bool finish)
{
    // qCDebug(KArchiveLog) << "Calling ZSTD_compressStream2 with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
    const size_t result = ZSTD_compressStream2(d->cStream, &d->outBuffer, &d->inBuffer, finish ? ZSTD_e_end : ZSTD_e_continue);
    if (ZSTD_isError(result)) {
        return KFilterBase::Error;
    }

    return finish && result == 0 ? KFilterBase::End : KFilterBase::Ok;
}

#endif
