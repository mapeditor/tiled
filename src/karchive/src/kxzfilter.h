/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2007-2008 Per Øyvind Karlsen <peroyvind@mandriva.org>

   Based on kbzip2filter:
   SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KXZFILTER_H
#define KXZFILTER_H

#include <config-compression.h>

#if HAVE_XZ_SUPPORT

#include "kfilterbase.h"

/**
 * Internal class used by KFilterDev
 * @internal
 */
class KXzFilter : public KFilterBase
{
public:
    KXzFilter();
    virtual ~KXzFilter();

    bool init(int) override;

    enum Flag {
        AUTO = 0,
        LZMA = 1,
        LZMA2 = 2,
        BCJ = 3, //X86
        POWERPC = 4,
        IA64 = 5,
        ARM = 6,
        ARMTHUMB = 7,
        SPARC = 8
    };

    virtual bool init(int, Flag flag, const QVector<unsigned char> &props);
    int mode() const override;
    bool terminate() override;
    void reset() override;
    bool readHeader() override
    {
        return true;    // lzma handles it by itself ! Cool !
    }
    bool writeHeader(const QByteArray &) override
    {
        return true;
    }
    void setOutBuffer(char *data, uint maxlen) override;
    void setInBuffer(const char *data, uint size) override;
    int  inBufferAvailable() const override;
    int  outBufferAvailable() const override;
    Result uncompress() override;
    Result compress(bool finish) override;
private:
    class Private;
    Private *const d;
};

#endif

#endif // KXZFILTER_H
