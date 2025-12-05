/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>

   Moved from ktar.h by Roberto Teixeira <maragato@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KARCHIVEDIRECTORY_H
#define KARCHIVEDIRECTORY_H

#include <sys/stat.h>
#include <sys/types.h>

#include <QDate>
#include <QString>
#include <QStringList>

#include <karchiveentry.h>

class KArchiveDirectoryPrivate;
class KArchiveFile;
/*!
 * \class KArchiveDirectory
 * \inmodule KArchive
 *
 * \brief A directory in an archive.
 *
 * \sa KArchive
 * \sa KArchiveFile
 */
class KARCHIVE_EXPORT KArchiveDirectory : public KArchiveEntry
{
public:
    /*!
     * Creates a new directory entry.
     *
     * \a archive the entries archive
     *
     * \a name the name of the entry
     *
     * \a access the permissions in unix format
     *
     * \a date the date (in seconds since 1970)
     *
     * \a user the user that owns the entry
     *
     * \a group the group that owns the entry
     *
     * \a symlink the symlink, or QString()
     */
    KArchiveDirectory(KArchive *archive,
                      const QString &name,
                      int access,
                      const QDateTime &date,
                      const QString &user,
                      const QString &group,
                      const QString &symlink);

    ~KArchiveDirectory() override;

    /*!
     * Returns a list of sub-entries.
     *
     * Note that the list is not sorted, it's even in random order (due to using a hashtable).
     * Use sort() on the result to sort the list by filename.
     *
     * Returns the names of all entries in this directory (filenames, no path).
     */
    QStringList entries() const;

    /*!
     * Returns the entry in the archive with the given name.
     *
     * The entry could be a file or a directory, use isFile() to find out which one it is.
     *
     * \a name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
     *
     * Returns a pointer to the entry in the directory, or a null pointer if there is no such entry.
     */
    const KArchiveEntry *entry(const QString &name) const;

    /*!
     * Returns the file entry in the archive with the given name.
     *
     * If the entry exists and is a file, a KArchiveFile is returned.
     *
     * Otherwise, a null pointer is returned.
     *
     * This is a convenience method for entry(), when we know the entry is expected to be a file.
     *
     *
     * \a name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
     *
     * Returns a pointer to the file entry in the directory, or a null pointer if there is no such file entry.
     * \since 5.3
     */
    const KArchiveFile *file(const QString &name) const;

#if KARCHIVE_ENABLE_DEPRECATED_SINCE(6, 13)
    /*!
     * \internal
     * Adds a new entry to the directory.
     *
     * Note: this can delete the entry if another one with the same name is already present
     *
     * \deprecated[6.13]
     *
     * Use addEntryV2() instead.
     */
    KARCHIVE_DEPRECATED_VERSION(6, 13, "Use addEntryV2() instead.")
    void addEntry(KArchiveEntry *); // KF7 TODO: remove
#endif

    /*!
     * \internal
     * Adds a new entry to the directory.
     *
     * Returns whether the entry was added or not. Non added entries are deleted
     * \since 6.13
     *
     * Returns whether the entry was added or not. Non added entries are deleted
     */
    [[nodiscard]] bool addEntryV2(KArchiveEntry *); // KF7 TODO: rename to addEntry

#if KARCHIVE_ENABLE_DEPRECATED_SINCE(6, 13)
    /*!
     * \internal
     *
     * Removes an entry from the directory.
     *
     * \deprecated[6.13]
     * Use removeEntryV2() instead.
     */
    KARCHIVE_DEPRECATED_VERSION(6, 13, "Use removeEntryV2() instead.")
    void removeEntry(KArchiveEntry *); // KF7 TODO: remove
#endif

    /*!
     * Removes an entry from the directory.
     *
     * Returns whether the entry was removed or not.
     * \since 6.13
     */
    [[nodiscard]] bool removeEntryV2(KArchiveEntry *); // KF7 TODO: rename to removeEntry

    /*
     * Returns true, since this entry is a directory
     */
    bool isDirectory() const override;

    /*!
     * Extracts all entries in this archive directory to the directory
     *
     * \a dest.
     *
     * \a dest the directory to extract to
     *
     * \a recursive if set to true, subdirectories are extracted as well
     *
     * Returns true on success, false if the directory (dest + '/' + name()) couldn't be created
     */
    bool copyTo(const QString &dest, bool recursive = true) const;

protected:
    void virtual_hook(int id, void *data) override;

private:
    friend class KArchiveDirectoryPrivate;
    KArchiveDirectoryPrivate *const d;
};

#endif
