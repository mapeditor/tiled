/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2001, 2002, 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "klimitediodevice_p.h"
#include "loggingcategory.h"

#ifdef TEST_MODE
#define WARNING qWarning()
#else
#define WARNING qCWarning(KArchiveLog)
#endif

KLimitedIODevice::KLimitedIODevice(QIODevice *dev, qint64 start, qint64 length)
    : m_dev(dev)
    , m_start(start)
    , m_length(length)
{
    // qCDebug(KArchiveLog) << "start=" << start << "length=" << length;

    const bool res = open(QIODevice::ReadOnly); // krazy:exclude=syscalls
    
    // KLimitedIODevice always returns true
    Q_ASSERT(res);

    if(!res) {
        WARNING << "failed to open LimitedIO device for reading.";
    }
}

bool KLimitedIODevice::open(QIODevice::OpenMode m)
{
    // qCDebug(KArchiveLog) << "m=" << m;
    if (m & QIODevice::ReadOnly) {
        /*bool ok = false;
          if ( m_dev->isOpen() )
          ok = ( m_dev->mode() == QIODevice::ReadOnly );
          else
          ok = m_dev->open( m );
          if ( ok )*/
        m_dev->seek(m_start); // No concurrent access !
    } else {
        WARNING << "KLimitedIODevice::open only supports QIODevice::ReadOnly!";
    }
    setOpenMode(QIODevice::ReadOnly);
    return true;
}

void KLimitedIODevice::close()
{
}

qint64 KLimitedIODevice::size() const
{
    return m_length;
}

qint64 KLimitedIODevice::readData(char *data, qint64 maxlen)
{
    maxlen = qMin(maxlen, m_length - pos()); // Apply upper limit
    return m_dev->read(data, maxlen);
}

bool KLimitedIODevice::seek(qint64 pos)
{
    Q_ASSERT(pos <= m_length);
    pos = qMin(pos, m_length); // Apply upper limit
    bool ret = m_dev->seek(m_start + pos);
    if (ret) {
        QIODevice::seek(pos);
    }
    return ret;
}

qint64 KLimitedIODevice::bytesAvailable() const
{
    return QIODevice::bytesAvailable();
}

bool KLimitedIODevice::isSequential() const
{
    return m_dev->isSequential();
}

#include "moc_klimitediodevice_p.cpp"
