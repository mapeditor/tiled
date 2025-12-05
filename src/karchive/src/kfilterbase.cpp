/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kfilterbase.h"

#include <QIODevice>

class KFilterBasePrivate
{
public:
    KFilterBasePrivate()
        : m_flags(KFilterBase::WithHeaders)
        , m_dev(nullptr)
        , m_bAutoDel(false)
    {
    }
    KFilterBase::FilterFlags m_flags;
    QIODevice *m_dev;
    bool m_bAutoDel;
};

KFilterBase::KFilterBase()
    : d(new KFilterBasePrivate)
{
}

KFilterBase::~KFilterBase()
{
    if (d->m_bAutoDel) {
        delete d->m_dev;
    }
    delete d;
}

void KFilterBase::setDevice(QIODevice *dev, bool autodelete)
{
    d->m_dev = dev;
    d->m_bAutoDel = autodelete;
}

QIODevice *KFilterBase::device()
{
    return d->m_dev;
}

bool KFilterBase::inBufferEmpty() const
{
    return inBufferAvailable() == 0;
}

bool KFilterBase::outBufferFull() const
{
    return outBufferAvailable() == 0;
}

bool KFilterBase::terminate()
{
    return true;
}

void KFilterBase::reset()
{
}

void KFilterBase::setFilterFlags(FilterFlags flags)
{
    d->m_flags = flags;
}

KFilterBase::FilterFlags KFilterBase::filterFlags() const
{
    return d->m_flags;
}

void KFilterBase::virtual_hook(int, void *)
{
    /*BASE::virtual_hook( id, data );*/
}
