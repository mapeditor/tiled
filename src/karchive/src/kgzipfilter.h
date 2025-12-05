/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000, 2009 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __kgzipfilter__h
#define __kgzipfilter__h

#include "kfilterbase.h"

/*!
 * Internal class used by KCompressionDevice
 *
 * This header is not installed.
 *
 * \internal
 */
class KGzipFilter : public KFilterBase
{
public:
    KGzipFilter();
    ~KGzipFilter() override;

    bool init(int mode) override;

    // The top of zlib.h explains it: there are three cases.
    // - Raw deflate, no header (e.g. inside a ZIP file)
    // - Thin zlib header (1) (which is normally what HTTP calls "deflate" (2))
    // - Gzip header, implemented here by readHeader
    //
    // (1) as written out by compress()/compress2()
    // (2) see https://www.zlib.net/zlib_faq.html#faq39
    enum Flag {
        RawDeflate = 0, // raw deflate data
        ZlibHeader = 1, // zlib headers (HTTP deflate)
        GZipHeader = 2,
    };
    bool init(int mode, Flag flag); // for direct users of KGzipFilter
    int mode() const override;
    bool terminate() override;
    void reset() override;
    bool readHeader() override; // this is about the GZIP header
    bool writeHeader(const QByteArray &fileName) override;
    void writeFooter();
    void setOutBuffer(char *data, uint maxlen) override;
    void setInBuffer(const char *data, uint size) override;
    int inBufferAvailable() const override;
    int outBufferAvailable() const override;
    Result uncompress() override;
    Result compress(bool finish) override;

private:
    Result uncompress_noop();
    class Private;
    Private *const d;
};

#endif
