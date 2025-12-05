/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2021 Albert Astals Cid <aacid@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KZSTDFILTER_H
#define KZSTDFILTER_H

#include <config-compression.h>

#if HAVE_ZSTD_SUPPORT

#include "kfilterbase.h"

#include <memory>

/*!
 * Internal class used by KCompressionDevice
 * \internal
 */
class KZstdFilter : public KFilterBase
{
public:
    KZstdFilter();
    ~KZstdFilter() override;

    bool init(int) override;
    int mode() const override;
    bool terminate() override;
    void reset() override;
    bool readHeader() override
    {
        return true;
    }
    bool writeHeader(const QByteArray &) override
    {
        return true;
    }
    void setOutBuffer(char *data, uint maxlen) override;
    void setInBuffer(const char *data, uint size) override;
    int inBufferAvailable() const override;
    int outBufferAvailable() const override;
    Result uncompress() override;
    Result compress(bool finish) override;

private:
    class Private;
    const std::unique_ptr<Private> d;
};

#endif

#endif
