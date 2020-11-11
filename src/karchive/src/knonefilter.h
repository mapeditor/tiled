/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

   Based on kbzip2filter:
   SPDX-FileCopyrightText: 2000, 2009 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __knonefilter__h
#define __knonefilter__h

#include "kfilterbase.h"

/**
 * Internal class used by KFilterDev
 *
 * This header is not installed.
 *
 * @internal
 */
class KNoneFilter : public KFilterBase
{
public:
    KNoneFilter();
    virtual ~KNoneFilter();

    bool init(int mode) override;
    int mode() const override;
    bool terminate() override;
    void reset() override;
    bool readHeader() override; // this is about the GZIP header
    bool writeHeader(const QByteArray &fileName) override;
    void setOutBuffer(char *data, uint maxlen) override;
    void setInBuffer(const char *data, uint size) override;
    int  inBufferAvailable() const override;
    int  outBufferAvailable() const override;
    Result uncompress() override;
    Result compress(bool finish) override;

private:
    Result copyData();

    class Private;
    Private *const d;
};

#endif
