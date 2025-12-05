/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>

   Moved from ktar.h by Roberto Teixeira <maragato@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KARCHIVEFILE_H
#define KARCHIVEFILE_H

#include <karchiveentry.h>

class KArchiveFilePrivate;
/*!
 * \class KArchiveFile
 * \inmodule KArchive
 *
 * \brief A file in an archive.
 *
 * \sa KArchive
 * \sa KArchiveDirectory
 */
class KARCHIVE_EXPORT KArchiveFile : public KArchiveEntry
{
public:
    /*!
     * Creates a new file entry. Do not call this, KArchive takes care of it.
     * \a archive the entries archive
     * \a name the name of the entry
     * \a access the permissions in unix format
     * \a date the date (in seconds since 1970)
     * \a user the user that owns the entry
     * \a group the group that owns the entry
     * \a symlink the symlink, or QString()
     * \a pos the position of the file in the directory
     * \a size the size of the file
     */
    KArchiveFile(KArchive *archive,
                 const QString &name,
                 int access,
                 const QDateTime &date,
                 const QString &user,
                 const QString &group,
                 const QString &symlink,
                 qint64 pos,
                 qint64 size);

    /*!
     * Destructor. Do not call this, KArchive takes care of it.
     */
    ~KArchiveFile() override;

    /*!
     * Position of the data in the [uncompressed] archive.
     * Returns the position of the file
     */
    qint64 position() const;
    /*!
     * Size of the data.
     * Returns the size of the file
     */
    qint64 size() const;
    /*!
     * Set size of data, usually after writing the file.
     * \a s the new size of the file
     */
    void setSize(qint64 s);

    /*!
     * Returns the content of this file.
     *
     * \note The data returned by this call is not cached.
     *
     * \warning This method loads the entire file content into memory at once. For large files or untrusted archives, this could cause excessive memory
     * allocation. Consider reading in chunks using createDevice() instead when dealing with archives from untrusted sources.
     */
    virtual QByteArray data() const;

    /*!
     * This method returns QIODevice (internal class: KLimitedIODevice)
     * on top of the underlying QIODevice. This is obviously for reading only.
     *
     * WARNING: Note that the ownership of the device is being transferred to the caller,
     * who will have to delete it.
     *
     * The returned device auto-opens (in readonly mode), no need to open it.
     * Returns the QIODevice of the file
     */
    virtual QIODevice *createDevice() const;

    /*!
     * Checks whether this entry is a file.
     * Returns true, since this entry is a file
     */
    bool isFile() const override;

    /*!
     * Extracts the file to the directory \a dest
     * \a dest the directory to extract to
     * Returns true on success, false if the file (dest + '/' + name()) couldn't be created
     */
    bool copyTo(const QString &dest) const;

protected:
    void virtual_hook(int id, void *data) override;

private:
    KArchiveFilePrivate *const d;
};

#endif
