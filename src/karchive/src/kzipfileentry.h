/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2002 Holger Schroeder <holger-kde@holgis.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KZIPFILEENTRY_H
#define KZIPFILEENTRY_H

#include "karchive.h"

class KZip;
/**
 * @class KZipFileEntry kzipfileentry.h KZipFileEntry
 *
 * A KZipFileEntry represents a file in a zip archive.
 */
class KARCHIVE_EXPORT KZipFileEntry : public KArchiveFile
{
public:
    /**
     * Creates a new zip file entry. Do not call this, KZip takes care of it.
     */
    KZipFileEntry(KZip *zip, const QString &name, int access, const QDateTime &date,
                  const QString &user, const QString &group, const QString &symlink,
                  const QString &path, qint64 start, qint64 uncompressedSize,
                  int encoding, qint64 compressedSize);

    /**
     * Destructor. Do not call this.
     */
    ~KZipFileEntry();

    int encoding() const;
    qint64 compressedSize() const;

    /// Only used when writing
    void setCompressedSize(qint64 compressedSize);

    /// Header start: only used when writing
    void setHeaderStart(qint64 headerstart);
    qint64 headerStart() const;

    /// CRC: only used when writing
    unsigned long crc32() const;
    void setCRC32(unsigned long crc32);

    /// Name with complete path - KArchiveFile::name() is the filename only (no path)
    const QString &path() const;

    /**
     * @return the content of this file.
     * Call data() with care (only once per file), this data isn't cached.
     */
    QByteArray data() const override;

    /**
     * This method returns a QIODevice to read the file contents.
     * This is obviously for reading only.
     * Note that the ownership of the device is being transferred to the caller,
     * who will have to delete it.
     * The returned device auto-opens (in readonly mode), no need to open it.
     */
    QIODevice *createDevice() const override;

private:
    class KZipFileEntryPrivate;
    KZipFileEntryPrivate *const d;
};

#endif
