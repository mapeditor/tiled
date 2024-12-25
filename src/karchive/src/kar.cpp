/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2002 Laurence Anderson <l.d.anderson@warwick.ac.uk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kar.h"
#include "karchive_p.h"
#include "loggingcategory.h"

#include <QFile>
#include <QDebug>

#include <limits>

#include "kfilterdev.h"
//#include "klimitediodevice_p.h"

// As documented in QByteArray
static constexpr int kMaxQByteArraySize = std::numeric_limits<int>::max() - 32;

////////////////////////////////////////////////////////////////////////
/////////////////////////// KAr ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class Q_DECL_HIDDEN KAr::KArPrivate
{
public:
    KArPrivate()
    {
    }
};

KAr::KAr(const QString &filename)
    : KArchive(filename)
    , d(new KArPrivate)
{
}

KAr::KAr(QIODevice *dev)
    : KArchive(dev)
    , d(new KArPrivate)
{
}

KAr::~KAr()
{
    if (isOpen()) {
        close();
    }
    delete d;
}

bool KAr::doPrepareWriting(const QString &, const QString &, const QString &,
                           qint64, mode_t, const QDateTime &, const QDateTime &, const QDateTime &)
{
    setErrorString(tr("Cannot write to AR file"));
    qCWarning(KArchiveLog) << "doPrepareWriting not implemented for KAr";
    return false;
}

bool KAr::doFinishWriting(qint64)
{
    setErrorString(tr("Cannot write to AR file"));
    qCWarning(KArchiveLog) << "doFinishWriting not implemented for KAr";
    return false;
}

bool KAr::doWriteDir(const QString &, const QString &, const QString &,
                     mode_t, const QDateTime &, const QDateTime &, const QDateTime &)
{
    setErrorString(tr("Cannot write to AR file"));
    qCWarning(KArchiveLog) << "doWriteDir not implemented for KAr";
    return false;
}

bool KAr::doWriteSymLink(const QString &, const QString &, const QString &,
                         const QString &, mode_t, const QDateTime &, const QDateTime &, const QDateTime &)
{
    setErrorString(tr("Cannot write to AR file"));
    qCWarning(KArchiveLog) << "doWriteSymLink not implemented for KAr";
    return false;
}

bool KAr::openArchive(QIODevice::OpenMode mode)
{
    // Open archive

    if (mode == QIODevice::WriteOnly) {
        return true;
    }
    if (mode != QIODevice::ReadOnly && mode != QIODevice::ReadWrite) {
        setErrorString(tr("Unsupported mode %1").arg(mode));
        return false;
    }

    QIODevice *dev = device();
    if (!dev) {
        return false;
    }

    QByteArray magic = dev->read(7);
    if (magic != "!<arch>") {
        setErrorString(tr("Invalid main magic"));
        return false;
    }

    QByteArray ar_longnames;
    while (! dev->atEnd()) {
        QByteArray ar_header;
        ar_header.resize(60);

        dev->seek(dev->pos() + (2 - (dev->pos() % 2)) % 2);   // Ar headers are padded to byte boundary

        if (dev->read(ar_header.data(), 60) != 60) {   // Read ar header
            qCWarning(KArchiveLog) << "Couldn't read header";
            return true; // Probably EOF / trailing junk
        }

        if (!ar_header.endsWith("`\n")) { // Check header magic // krazy:exclude=strings
            setErrorString(tr("Invalid magic"));
            return false;
        }

        QByteArray name = ar_header.mid(0, 16);   // Process header
        const int date = ar_header.mid(16, 12).trimmed().toInt();
        //const int uid = ar_header.mid( 28, 6 ).trimmed().toInt();
        //const int gid = ar_header.mid( 34, 6 ).trimmed().toInt();
        const int mode = ar_header.mid(40, 8).trimmed().toInt(nullptr, 8);
        const qint64 size = ar_header.mid(48, 10).trimmed().toInt();
        if (size < 0 || size > kMaxQByteArraySize) {
            setErrorString(tr("Invalid size"));
            return false;
        }

        bool skip_entry = false; // Deal with special entries
        if (name.mid(0, 1) == "/") {
            if (name.mid(1, 1) == "/") { // Longfilename table entry
                ar_longnames.resize(size);
                // Read the table. Note that the QByteArray will contain NUL characters after each entry.
                dev->read(ar_longnames.data(), size);
                skip_entry = true;
                qCDebug(KArchiveLog) << "Read in longnames entry";
            } else if (name.mid(1, 1) == " ") { // Symbol table entry
                qCDebug(KArchiveLog) << "Skipped symbol entry";
                dev->seek(dev->pos() + size);
                skip_entry = true;
            } else { // Longfilename, look it up in the table
                const int ar_longnamesIndex = name.mid(1, 15).trimmed().toInt();
                qCDebug(KArchiveLog) << "Longfilename #" << ar_longnamesIndex;
                if (ar_longnames.isEmpty()) {
                    setErrorString(tr("Invalid longfilename reference"));
                    return false;
                }
                if (ar_longnamesIndex < 0 || ar_longnamesIndex >= ar_longnames.size()) {
                    setErrorString(tr("Invalid longfilename position reference"));
                    return false;
                }
                name = QByteArray(ar_longnames.constData() + ar_longnamesIndex);
                name.truncate(name.indexOf('/'));
            }
        }
        if (skip_entry) {
            continue;
        }

        // Process filename
        name = name.trimmed();
        name.replace('/', QByteArray());
        qCDebug(KArchiveLog) << "Filename: " << name << " Size: " << size;

        KArchiveEntry *entry = new KArchiveFile(this, QString::fromLocal8Bit(name.constData()), mode, KArchivePrivate::time_tToDateTime(date),
                                                rootDir()->user(), rootDir()->group(), /*symlink*/ QString(),
                                                dev->pos(), size);
        rootDir()->addEntry(entry); // Ar files don't support directories, so everything in root

        dev->seek(dev->pos() + size);   // Skip contents
    }

    return true;
}

bool KAr::closeArchive()
{
    // Close the archive
    return true;
}

void KAr::virtual_hook(int id, void *data)
{
    KArchive::virtual_hook(id, data);
}
