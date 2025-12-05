/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "klzfilter.h"
#include "loggingcategory.h"

#include <QDebug>
#include <QIODevice>
#include <QLoggingCategory>

#if HAVE_XZ_SUPPORT

extern "C" {
#include <lzma.h>
}

#if LZMA_VERSION_MAJOR > 5 || (LZMA_VERSION_MAJOR == 5 && LZMA_VERSION_MINOR >= 4)
#define LZMA_LZIP_DECODER_AVAILABLE 1
#endif

#ifndef LZMA_LZIP_DECODER_AVAILABLE
static const uint32_t minDictSize = 1 << 12; // 4 KiB
static const uint32_t maxDictSize = 1 << 29; // 512 MiB
#endif

// Default dictionary size (8 MiB)
static const uint8_t defaultEncodedDictSize = 23;
static const uint32_t defaultDecodedDictSize = 1 << 23;

class Q_DECL_HIDDEN KLzFilter::Private
{
public:
    Private()
        : zStream(LZMA_STREAM_INIT)
        , mode(0)
        , isInitialized(false)
    {
    }

    lzma_stream zStream;

    uint32_t decodedDictSize;
    uint32_t crc32;

    int mode;
    bool isInitialized;
};

KLzFilter::KLzFilter()
    : d(new Private)
{
}

KLzFilter::~KLzFilter()
{
}

bool KLzFilter::init(int mode)
{
    if (d->isInitialized) {
        terminate();
    }

    d->zStream.next_in = nullptr;
    d->zStream.avail_in = 0;

    d->crc32 = 0;
    d->decodedDictSize = defaultDecodedDictSize;

    lzma_ret result;

    if (mode == QIODevice::ReadOnly) {
#ifdef LZMA_LZIP_DECODER_AVAILABLE
        result = lzma_lzip_decoder(&d->zStream, 100 << 20, 0);
        if (result != LZMA_OK) {
            qCWarning(KArchiveLog) << "lzma_lzip_decoder returned" << result;
            return false;
        }
        d->isInitialized = true;
#else
        // We cannot initialize lzma_raw_decoder here because we
        // need to read the header first to extract the dictionary size.
#endif
    } else if (mode == QIODevice::WriteOnly) {
        lzma_options_lzma lzma_opt;
        lzma_lzma_preset(&lzma_opt, LZMA_PRESET_DEFAULT);
        lzma_opt.dict_size = defaultDecodedDictSize;

        lzma_filter filters[2];
        filters[0].id = LZMA_FILTER_LZMA1;
        filters[0].options = &lzma_opt;
        filters[1].id = LZMA_VLI_UNKNOWN;
        filters[1].options = nullptr;

        result = lzma_raw_encoder(&d->zStream, filters);
        if (result != LZMA_OK) {
            qCWarning(KArchiveLog) << "lzma_raw_encoder returned" << result;
            return false;
        }
        d->isInitialized = true;
    } else {
        return false;
    }
    d->mode = mode;
    return true;
}

int KLzFilter::mode() const
{
    return d->mode;
}

bool KLzFilter::terminate()
{
    if (d->mode != QIODevice::ReadOnly && d->mode != QIODevice::WriteOnly) {
        return false;
    }

    if (d->isInitialized) {
        lzma_end(&d->zStream);
    }

    d->isInitialized = false;
    return true;
}

void KLzFilter::reset()
{
    terminate();
    init(d->mode);
}

void KLzFilter::setOutBuffer(char *data, uint maxlen)
{
    d->zStream.avail_out = maxlen;
    d->zStream.next_out = (uint8_t *)data;
}

void KLzFilter::setInBuffer(const char *data, unsigned int size)
{
    d->zStream.avail_in = size;
    d->zStream.next_in = (uint8_t *)const_cast<char *>(data);
}

int KLzFilter::inBufferAvailable() const
{
    return d->zStream.avail_in;
}

int KLzFilter::outBufferAvailable() const
{
    return d->zStream.avail_out;
}

#ifndef LZMA_LZIP_DECODER_AVAILABLE
static uint32_t parseUi32(const uint8_t *buffer)
{
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        value |= (uint32_t)buffer[i] << (i * 8);
    }
    return value;
}

static uint64_t parseUi64(const uint8_t *buffer)
{
    return (parseUi32(buffer) | (uint64_t)parseUi32(buffer + 4) << 32);
}
#endif

static void putUi32(uint8_t *buffer, uint32_t value)
{
    for (int i = 0; i < 4; ++i) {
        buffer[i] = value;
        value >>= 8;
    }
}

static void putUi64(uint8_t *buffer, uint64_t value)
{
    for (int i = 0; i < 8; ++i) {
        buffer[i] = value;
        value >>= 8;
    }
}

bool KLzFilter::readHeader()
{
    if (d->mode != QIODevice::ReadOnly) {
        return false;
    }

#ifndef LZMA_LZIP_DECODER_AVAILABLE
    if (d->zStream.avail_in < 6) {
        qCWarning(KArchiveLog) << "Not enough data to read LZIP header";
        return false;
    }

    const uint8_t *header = d->zStream.next_in;

    // check the lzip magic + version (should be 1)
    if (memcmp(header, "LZIP\x01", 5) != 0) {
        qCWarning(KArchiveLog) << "Invalid LZIP header or unsupported version";
        return false;
    }

    uint32_t dictSize = 1 << (header[5] & 0x1F);
    if (dictSize > minDictSize) {
        dictSize -= (dictSize / 16) * ((header[5] >> 5) & 7);
    }

    if (dictSize < minDictSize || dictSize > maxDictSize) {
        qCWarning(KArchiveLog) << "Invalid LZIP dictSize:" << dictSize;
        return false;
    }

    d->decodedDictSize = dictSize;

    d->zStream.next_in += 6;
    d->zStream.avail_in -= 6;
#endif

    return true;
}

bool KLzFilter::readTrailer()
{
    if (d->mode != QIODevice::ReadOnly) {
        return false;
    }

#ifndef LZMA_LZIP_DECODER_AVAILABLE
    if (d->zStream.avail_in < 20) {
        qCWarning(KArchiveLog) << "Not enough data to read LZIP header";
        return false;
    }

    uint64_t actualDataSize = d->zStream.total_out; // total uncompressed data
    uint64_t actualMemberSize = d->zStream.total_in + 26; // header (6) + data + trailer (20)

    const uint8_t *trailer = d->zStream.next_in;

    const uint32_t crc32 = parseUi32(trailer);
    if (crc32 != d->crc32) {
        qCWarning(KArchiveLog) << "Invalid LZIP CRC32:" << crc32;
        return false;
    }

    const uint64_t dataSize = parseUi64(trailer + 4);
    if (dataSize != actualDataSize) {
        qCWarning(KArchiveLog) << "Invalid LZIP dataSize:" << dataSize;
        return false;
    }

    const uint64_t memberSize = parseUi64(trailer + 12);
    if (memberSize != actualMemberSize) {
        qCWarning(KArchiveLog) << "Invalid LZIP memberSize:" << memberSize;
        return false;
    }

    d->zStream.next_in += 20;
    d->zStream.avail_in -= 20;
#endif

    return true;
}

bool KLzFilter::writeHeader(const QByteArray &)
{
    if (d->mode != QIODevice::WriteOnly) {
        return false;
    }

    if (d->zStream.avail_out < 6) {
        qCWarning(KArchiveLog) << "Not enough space to write LZIP header";
        return false;
    }

    d->zStream.next_out[0] = 'L';
    d->zStream.next_out[1] = 'Z';
    d->zStream.next_out[2] = 'I';
    d->zStream.next_out[3] = 'P';
    d->zStream.next_out[4] = 1; // version = 1
    d->zStream.next_out[5] = defaultEncodedDictSize;

    d->zStream.next_out += 6;
    d->zStream.avail_out -= 6;

    return true;
}

bool KLzFilter::writeTrailer()
{
    if (d->mode != QIODevice::WriteOnly) {
        return false;
    }

    if (d->zStream.avail_out < 20) {
        qCWarning(KArchiveLog) << "Not enough space to write LZIP trailer";
        return false;
    }

    uint64_t dataSize = d->zStream.total_in; // total uncompressed data
    uint64_t memberSize = d->zStream.total_out + 26; // header (6) + data + trailer (20)

    putUi32(d->zStream.next_out, d->crc32);
    putUi64(d->zStream.next_out + 4, dataSize);
    putUi64(d->zStream.next_out + 12, memberSize);

    d->zStream.next_out += 20;
    d->zStream.avail_out -= 20;

    return true;
}

KLzFilter::Result KLzFilter::uncompress()
{
#ifdef LZMA_LZIP_DECODER_AVAILABLE
    lzma_ret result = lzma_code(&d->zStream, LZMA_RUN);
#else
    lzma_ret result;
    if (!d->isInitialized) {
        lzma_options_lzma lzma_opt;
        lzma_lzma_preset(&lzma_opt, LZMA_PRESET_DEFAULT);
        lzma_opt.dict_size = d->decodedDictSize;

        lzma_filter filters[2];
        filters[0].id = LZMA_FILTER_LZMA1;
        filters[0].options = &lzma_opt;
        filters[1].id = LZMA_VLI_UNKNOWN;
        filters[1].options = nullptr;

        result = lzma_raw_decoder(&d->zStream, filters);
        if (result != LZMA_OK) {
            qCWarning(KArchiveLog) << "lzma_raw_decoder returned" << result;
            return KFilterBase::Error;
        }

        d->isInitialized = true;
    }

    size_t prevAvailOut = d->zStream.avail_out;
    result = lzma_code(&d->zStream, LZMA_RUN);
    size_t written = prevAvailOut - d->zStream.avail_out;

    if (written > 0) {
        d->crc32 = lzma_crc32(d->zStream.next_out - written, written, d->crc32);
    }
#endif

    switch (result) {
    case LZMA_OK:
        return KFilterBase::Ok;
    case LZMA_STREAM_END:
        if (!readTrailer()) {
            return KFilterBase::Error;
        }
        return KFilterBase::End;
    default:
        qCWarning(KArchiveLog) << "lzma_code returned" << result;
        return KFilterBase::Error;
    }
}

KLzFilter::Result KLzFilter::compress(bool finish)
{
    size_t prevAvailIn = d->zStream.avail_in;
    lzma_ret result = lzma_code(&d->zStream, finish ? LZMA_FINISH : LZMA_RUN);
    size_t read = prevAvailIn - d->zStream.avail_in;

    if (read > 0) {
        d->crc32 = lzma_crc32(d->zStream.next_in - read, read, d->crc32);
    }

    switch (result) {
    case LZMA_OK:
        return KFilterBase::Ok;
    case LZMA_STREAM_END:
        if (finish && !writeTrailer()) {
            return KFilterBase::Error;
        }
        return KFilterBase::End;
    default:
        qCDebug(KArchiveLog) << "  lzma_code returned " << result;
        return KFilterBase::Error;
    }
}

#endif
