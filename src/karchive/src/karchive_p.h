/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KARCHIVE_P_H
#define KARCHIVE_P_H

#include "karchive.h"

#include <QSaveFile>

// Documentation says that QByteArray should be able to hold up to 2^63 on 64 bit platforms
// but practice says it aborts with something like 2314885530818453536, so go with MAX_INT for now
static constexpr int kMaxQByteArraySize = std::numeric_limits<int>::max() - 32;

class KArchivePrivate
{
    Q_DECLARE_TR_FUNCTIONS(KArchivePrivate)

public:
    KArchivePrivate(KArchive *parent)
        : q(parent)
    {
    }
    ~KArchivePrivate()
    {
        if (deviceOwned) {
            delete dev; // we created it ourselves in open()
            dev = nullptr;
        }

        delete rootDir;
    }

    KArchivePrivate(const KArchivePrivate &) = delete;
    KArchivePrivate &operator=(const KArchivePrivate &) = delete;

    static bool hasRootDir(KArchive *archive)
    {
        return archive->d->rootDir;
    }

    void abortWriting();

    static QDateTime time_tToDateTime(uint seconds);

    KArchiveDirectory *findOrCreateDirectory(const QStringView path);

    KArchive *q = nullptr;
    KArchiveDirectory *rootDir = nullptr;
    std::unique_ptr<QSaveFile> saveFile;
    QIODevice *dev = nullptr;
    QString fileName;
    QIODevice::OpenMode mode = QIODevice::NotOpen;
    bool deviceOwned = false; // if true, we (KArchive) own dev and must delete it
    QString errorStr{tr("Unknown error")};
};

#endif // KARCHIVE_P_H
