/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

   Based on kbzip2filter:
   SPDX-FileCopyrightText: 2000, 2009 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knonefilter.h"

#include <QFile>

class Q_DECL_HIDDEN KNoneFilter::Private
{
public:
    Private()
        : mode(0)
        , avail_out(0)
        , avail_in(0)
        , next_in(nullptr)
        , next_out(nullptr)
    {
    }

    int mode;
    int avail_out;
    int avail_in;
    const char *next_in;
    char *next_out;
};

KNoneFilter::KNoneFilter()
    : d(new Private)
{
}

KNoneFilter::~KNoneFilter()
{
    delete d;
}

bool KNoneFilter::init(int mode)
{
    d->mode = mode;
    return true;
}

int KNoneFilter::mode() const
{
    return d->mode;
}

bool KNoneFilter::terminate()
{
    return true;
}

void KNoneFilter::reset()
{
}

bool KNoneFilter::readHeader()
{
    return true;
}

bool KNoneFilter::writeHeader(const QByteArray & /*fileName*/)
{
    return true;
}

void KNoneFilter::setOutBuffer(char *data, uint maxlen)
{
    d->avail_out = maxlen;
    d->next_out = data;
}

void KNoneFilter::setInBuffer(const char *data, uint size)
{
    d->next_in = data;
    d->avail_in = size;
}

int KNoneFilter::inBufferAvailable() const
{
    return d->avail_in;
}

int KNoneFilter::outBufferAvailable() const
{
    return d->avail_out;
}

KNoneFilter::Result KNoneFilter::uncompress()
{
#ifndef NDEBUG
    if (d->mode != QIODevice::ReadOnly) {
        return KFilterBase::Error;
    }
#endif
    return copyData();
}

KNoneFilter::Result KNoneFilter::compress(bool finish)
{
    Q_ASSERT(d->mode == QIODevice::WriteOnly);
    Q_UNUSED(finish);

    return copyData();
}

KNoneFilter::Result KNoneFilter::copyData()
{
    Q_ASSERT(d->avail_out > 0);
    if (d->avail_in > 0) {
        const int n = qMin(d->avail_in, d->avail_out);
        memcpy(d->next_out, d->next_in, n);
        d->avail_out -= n;
        d->next_in += n;
        d->next_out += n;
        d->avail_in -= n;
        return KFilterBase::Ok;
    } else {
        return KFilterBase::End;
    }
}
