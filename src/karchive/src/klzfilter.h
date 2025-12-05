/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLZFILTER_H
#define KLZFILTER_H

#include <config-compression.h>

#if HAVE_XZ_SUPPORT

#include "kfilterbase.h"

/**
 * Internal class used by KCompressionDevice
 * @internal
 */
class KLzFilter : public KFilterBase
{
public:
    KLzFilter();
    ~KLzFilter() override;

    bool init(int mode) override;
    int mode() const override;
    bool terminate() override;
    void reset() override;
    bool readHeader() override;
    bool readTrailer();
    bool writeHeader(const QByteArray &) override;
    bool writeTrailer();
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

#endif // HAVE_XZ_SUPPORT

#endif // KLZFILTER_H
