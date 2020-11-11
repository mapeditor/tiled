/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kgzipfilter.h"
#include "loggingcategory.h"

#include <time.h>

#include <zlib.h>
#include <QDebug>
#include <QIODevice>

/* gzip flag byte */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */

// #define DEBUG_GZIP

class Q_DECL_HIDDEN KGzipFilter::Private
{
public:
    Private()
        : headerWritten(false)
        , footerWritten(false)
        , compressed(false)
        , mode(0)
        , crc(0)
        , isInitialized(false)
    {
        zStream.zalloc = static_cast<alloc_func>(nullptr);
        zStream.zfree = static_cast<free_func>(nullptr);
        zStream.opaque = static_cast<voidpf>(nullptr);
    }

    z_stream zStream;
    bool headerWritten;
    bool footerWritten;
    bool compressed;
    int mode;
    ulong crc;
    bool isInitialized;
};

KGzipFilter::KGzipFilter()
    : d(new Private)
{
}

KGzipFilter::~KGzipFilter()
{
    delete d;
}

bool KGzipFilter::init(int mode)
{
    switch (filterFlags()) {
    case NoHeaders:
        return init(mode, RawDeflate);
    case WithHeaders:
        return init(mode, GZipHeader);
    case ZlibHeaders:
        return init(mode, ZlibHeader);
    }
    return false;
}

bool KGzipFilter::init(int mode, Flag flag)
{
    if (d->isInitialized) {
        terminate();
    }
    d->zStream.next_in = Z_NULL;
    d->zStream.avail_in = 0;
    if (mode == QIODevice::ReadOnly) {
        const int windowBits = (flag == RawDeflate)
                               ? -MAX_WBITS /*no zlib header*/
                               : (flag == GZipHeader) ?
                               MAX_WBITS + 32 /* auto-detect and eat gzip header */
                               : MAX_WBITS /*zlib header*/;
        const int result = inflateInit2(&d->zStream, windowBits);
        if (result != Z_OK) {
            //qCDebug(KArchiveLog) << "inflateInit2 returned " << result;
            return false;
        }
    } else if (mode == QIODevice::WriteOnly) {
        int result = deflateInit2(&d->zStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY); // same here
        if (result != Z_OK) {
            //qCDebug(KArchiveLog) << "deflateInit returned " << result;
            return false;
        }
    } else {
        //qCWarning(KArchiveLog) << "KGzipFilter: Unsupported mode " << mode << ". Only QIODevice::ReadOnly and QIODevice::WriteOnly supported";
        return false;
    }
    d->mode = mode;
    d->compressed = true;
    d->headerWritten = false;
    d->footerWritten = false;
    d->isInitialized = true;
    return true;
}

int KGzipFilter::mode() const
{
    return d->mode;
}

bool KGzipFilter::terminate()
{
    if (d->mode == QIODevice::ReadOnly) {
        int result = inflateEnd(&d->zStream);
        if (result != Z_OK) {
            //qCDebug(KArchiveLog) << "inflateEnd returned " << result;
            return false;
        }
    } else if (d->mode == QIODevice::WriteOnly) {
        int result = deflateEnd(&d->zStream);
        if (result != Z_OK) {
            //qCDebug(KArchiveLog) << "deflateEnd returned " << result;
            return false;
        }
    }
    d->isInitialized = false;
    return true;
}

void KGzipFilter::reset()
{
    if (d->mode == QIODevice::ReadOnly) {
        int result = inflateReset(&d->zStream);
        if (result != Z_OK) {
            //qCDebug(KArchiveLog) << "inflateReset returned " << result;
            // TODO return false
        }
    } else if (d->mode == QIODevice::WriteOnly) {
        int result = deflateReset(&d->zStream);
        if (result != Z_OK) {
            //qCDebug(KArchiveLog) << "deflateReset returned " << result;
            // TODO return false
        }
        d->headerWritten = false;
        d->footerWritten = false;
    }
}

bool KGzipFilter::readHeader()
{
    // We now rely on zlib to read the full header (see the MAX_WBITS + 32 in init).
    // We just use this method to check if the data is actually compressed.

#ifdef DEBUG_GZIP
    qCDebug(KArchiveLog) << "avail=" << d->zStream.avail_in;
#endif
    // Assume not compressed until we see a gzip header
    d->compressed = false;
    const Bytef *p = d->zStream.next_in;
    int i = d->zStream.avail_in;
    if ((i -= 10)  < 0) {
        return false;    // Need at least 10 bytes
    }
#ifdef DEBUG_GZIP
    qCDebug(KArchiveLog) << "first byte is " << QString::number(*p, 16);
#endif
    if (*p++ != 0x1f) {
        return false;    // GZip magic
    }
#ifdef DEBUG_GZIP
    qCDebug(KArchiveLog) << "second byte is " << QString::number(*p, 16);
#endif
    if (*p++ != 0x8b) {
        return false;
    }

    d->compressed = true;
#ifdef DEBUG_GZIP
    qCDebug(KArchiveLog) << "header OK";
#endif
    return true;
}

/* Output a 16 bit value, lsb first */
#define put_short(w) \
    *p++ = uchar((w) & 0xff); \
    *p++ = uchar(ushort(w) >> 8);

/* Output a 32 bit value to the bit stream, lsb first */
#define put_long(n) \
    put_short((n) & 0xffff); \
    put_short((ulong(n)) >> 16);

bool KGzipFilter::writeHeader(const QByteArray &fileName)
{
    Bytef *p = d->zStream.next_out;
    int i = d->zStream.avail_out;
    *p++ = 0x1f;
    *p++ = 0x8b;
    *p++ = Z_DEFLATED;
    *p++ = ORIG_NAME;
    put_long(time(nullptr));     // Modification time (in unix format)
    *p++ = 0; // Extra flags (2=max compress, 4=fastest compress)
    *p++ = 3; // Unix

    uint len = fileName.length();
    for (uint j = 0; j < len; ++j) {
        *p++ = fileName[j];
    }
    *p++ = 0;
    int headerSize = p - d->zStream.next_out;
    i -= headerSize;
    Q_ASSERT(i > 0);
    d->crc = crc32(0L, nullptr, 0);
    d->zStream.next_out = p;
    d->zStream.avail_out = i;
    d->headerWritten = true;
    return true;
}

void KGzipFilter::writeFooter()
{
    Q_ASSERT(d->headerWritten);
    Q_ASSERT(!d->footerWritten);
    Bytef *p = d->zStream.next_out;
    int i = d->zStream.avail_out;
    //qCDebug(KArchiveLog) << "avail_out=" << i << "writing CRC=" << QString::number(d->crc, 16) << "at p=" << p;
    put_long(d->crc);
    //qCDebug(KArchiveLog) << "writing totalin=" << d->zStream.total_in << "at p=" << p;
    put_long(d->zStream.total_in);
    i -= p - d->zStream.next_out;
    d->zStream.next_out = p;
    d->zStream.avail_out = i;
    d->footerWritten = true;
}

void KGzipFilter::setOutBuffer(char *data, uint maxlen)
{
    d->zStream.avail_out = maxlen;
    d->zStream.next_out = reinterpret_cast<Bytef *>(data);
}
void KGzipFilter::setInBuffer(const char *data, uint size)
{
#ifdef DEBUG_GZIP
    qCDebug(KArchiveLog) << "avail_in=" << size;
#endif
    d->zStream.avail_in = size;
    d->zStream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data));
}
int KGzipFilter::inBufferAvailable() const
{
    return d->zStream.avail_in;
}
int KGzipFilter::outBufferAvailable() const
{
    return d->zStream.avail_out;
}

KGzipFilter::Result KGzipFilter::uncompress_noop()
{
    // I'm not sure we really need support for that (uncompressed streams),
    // but why not, it can't hurt to have it. One case I can think of is someone
    // naming a tar file "blah.tar.gz" :-)
    if (d->zStream.avail_in > 0) {
        int n = (d->zStream.avail_in < d->zStream.avail_out) ? d->zStream.avail_in : d->zStream.avail_out;
        memcpy(d->zStream.next_out, d->zStream.next_in, n);
        d->zStream.avail_out -= n;
        d->zStream.next_in += n;
        d->zStream.avail_in -= n;
        return KFilterBase::Ok;
    } else {
        return KFilterBase::End;
    }
}

KGzipFilter::Result KGzipFilter::uncompress()
{
#ifndef NDEBUG
    if (d->mode == 0) {
        //qCWarning(KArchiveLog) << "mode==0; KGzipFilter::init was not called!";
        return KFilterBase::Error;
    } else if (d->mode == QIODevice::WriteOnly) {
        //qCWarning(KArchiveLog) << "uncompress called but the filter was opened for writing!";
        return KFilterBase::Error;
    }
    Q_ASSERT(d->mode == QIODevice::ReadOnly);
#endif

    if (!d->compressed) {
        return uncompress_noop();
    }

#ifdef DEBUG_GZIP
    qCDebug(KArchiveLog) << "Calling inflate with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
    qCDebug(KArchiveLog) << "    next_in=" << d->zStream.next_in;
#endif

    while (d->zStream.avail_in > 0) {
        int result = inflate(&d->zStream, Z_SYNC_FLUSH);

#ifdef DEBUG_GZIP
        qCDebug(KArchiveLog) << " -> inflate returned " << result;
        qCDebug(KArchiveLog) << " now avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
        qCDebug(KArchiveLog) << "     next_in=" << d->zStream.next_in;
#endif

        if (result == Z_OK) {
            return KFilterBase::Ok;
        }

        // We can't handle any other results
        if (result != Z_STREAM_END) {
            return KFilterBase::Error;
        }

        // It really was the end
        if (d->zStream.avail_in == 0) {
            return KFilterBase::End;
        }

        // Store before resetting
        Bytef *data = d->zStream.next_in; // This is increased appropriately by zlib beforehand
        uInt size = d->zStream.avail_in;

        // Reset the stream, if that fails we assume we're at the end
        if (!init(d->mode)) {
            return KFilterBase::End;
        }

        // Reset the data to where we left off
        d->zStream.next_in = data;
        d->zStream.avail_in = size;
    }

    return KFilterBase::End;
}

KGzipFilter::Result KGzipFilter::compress(bool finish)
{
    Q_ASSERT(d->compressed);
    Q_ASSERT(d->mode == QIODevice::WriteOnly);

    const Bytef *p = d->zStream.next_in;
    ulong len = d->zStream.avail_in;
#ifdef DEBUG_GZIP
    qCDebug(KArchiveLog) << "  calling deflate with avail_in=" << inBufferAvailable() << " avail_out=" << outBufferAvailable();
#endif
    const int result = deflate(&d->zStream, finish ? Z_FINISH : Z_NO_FLUSH);
    if (result != Z_OK && result != Z_STREAM_END) {
        //qCDebug(KArchiveLog) << "  deflate returned " << result;
    }
    if (d->headerWritten) {
        //qCDebug(KArchiveLog) << "Computing CRC for the next " << len - d->zStream.avail_in << " bytes";
        d->crc = crc32(d->crc, p, len - d->zStream.avail_in);
    }
    KGzipFilter::Result callerResult = result == Z_OK ? KFilterBase::Ok : (Z_STREAM_END ? KFilterBase::End : KFilterBase::Error);

    if (result == Z_STREAM_END && d->headerWritten && !d->footerWritten) {
        if (d->zStream.avail_out >= 8 /*footer size*/) {
            //qCDebug(KArchiveLog) << "finished, write footer";
            writeFooter();
        } else {
            // No room to write the footer (#157706/#188415), we'll have to do it on the next pass.
            //qCDebug(KArchiveLog) << "finished, but no room for footer yet";
            callerResult = KFilterBase::Ok;
        }
    }
    return callerResult;
}
