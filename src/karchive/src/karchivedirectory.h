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
/**
 * @class KArchiveDirectory karchivedirectory.h KArchiveDirectory
 *
 * Represents a directory entry in a KArchive.
 * @short A directory in an archive.
 *
 * @see KArchive
 * @see KArchiveFile
 */
class KARCHIVE_EXPORT KArchiveDirectory : public KArchiveEntry
{
public:
    /**
     * Creates a new directory entry.
     * @param archive the entries archive
     * @param name the name of the entry
     * @param access the permissions in unix format
     * @param date the date (in seconds since 1970)
     * @param user the user that owns the entry
     * @param group the group that owns the entry
     * @param symlink the symlink, or QString()
     */
    KArchiveDirectory(KArchive *archive, const QString &name, int access, const QDateTime &date,
                      const QString &user, const QString &group,
                      const QString &symlink);

    virtual ~KArchiveDirectory();

    /**
     * Returns a list of sub-entries.
     * Note that the list is not sorted, it's even in random order (due to using a hashtable).
     * Use sort() on the result to sort the list by filename.
     *
     * @return the names of all entries in this directory (filenames, no path).
     */
    QStringList entries() const;

    /**
     * Returns the entry in the archive with the given name.
     * The entry could be a file or a directory, use isFile() to find out which one it is.
     * @param name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
     * @return a pointer to the entry in the directory, or a null pointer if there is no such entry.
     */
    const KArchiveEntry *entry(const QString &name) const;

    /**
     * Returns the file entry in the archive with the given name.
     * If the entry exists and is a file, a KArchiveFile is returned.
     * Otherwise, a null pointer is returned.
     * This is a convenience method for entry(), when we know the entry is expected to be a file.
     *
     * @param name may be "test1", "mydir/test3", "mydir/mysubdir/test3", etc.
     * @return a pointer to the file entry in the directory, or a null pointer if there is no such file entry.
     * @since 5.3
     */
    const KArchiveFile *file(const QString &name) const;

    /**
     * @internal
     * Adds a new entry to the directory.
     * Note: this can delete the entry if another one with the same name is already present
     */
    void addEntry(KArchiveEntry *); // KF6 TODO: return bool

    /**
     * @internal
     * Adds a new entry to the directory.
     * @return whether the entry was added or not. Non added entries are deleted
     */
    bool addEntryV2(KArchiveEntry *); // KF6 TODO: merge with the one above

    /**
     * @internal
     * Removes an entry from the directory.
     */
    void removeEntry(KArchiveEntry *); // KF6 TODO: return bool since it can fail

    /**
     * Checks whether this entry is a directory.
     * @return true, since this entry is a directory
     */
    bool isDirectory() const override;

    /**
     * Extracts all entries in this archive directory to the directory
     * @p dest.
     * @param dest the directory to extract to
     * @param recursive if set to true, subdirectories are extracted as well
     * @return true on success, false if the directory (dest + '/' + name()) couldn't be created
     */
    bool copyTo(const QString &dest, bool recursive = true) const;

protected:
    void virtual_hook(int id, void *data) override;
private:
    friend class KArchiveDirectoryPrivate;
    KArchiveDirectoryPrivate *const d;
};

#endif
