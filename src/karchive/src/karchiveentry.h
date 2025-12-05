/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>

   Moved from ktar.h by Roberto Teixeira <maragato@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KARCHIVEENTRY_H
#define KARCHIVEENTRY_H

#include <sys/stat.h>
#include <sys/types.h>

#include <karchive_export.h>

#include <QDateTime>
#include <QString>

#ifdef Q_OS_WIN
#include <qplatformdefs.h> // mode_t
#endif

class KArchiveDirectory;
class KArchiveFile;
class KArchive;

class KArchiveEntryPrivate;
/*!
 * \class KArchiveEntry
 * \inmodule KArchive
 *
 * \brief Base class for the archive-file's directory structure.
 *
 * \sa KArchiveFile
 * \sa KArchiveDirectory
 */
class KARCHIVE_EXPORT KArchiveEntry
{
public:
    /*!
     * Creates a new entry.
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
    KArchiveEntry(KArchive *archive, const QString &name, int access, const QDateTime &date, const QString &user, const QString &group, const QString &symlink);

    virtual ~KArchiveEntry();

    /*!
     * Creation date of the file.
     *
     * Returns the creation date
     */
    QDateTime date() const;

    /*!
     * Name of the file without path.
     *
     * Returns the file name without path
     */
    QString name() const;
    /*!
     * The permissions and mode flags as returned by the stat() function
     * in st_mode.
     *
     * Returns the permissions
     */
    mode_t permissions() const;
    /*!
     * User who created the file.
     *
     * Returns the owner of the file
     */
    QString user() const;
    /*!
     * Group of the user who created the file.
     *
     * Returns the group of the file
     */
    QString group() const;

    /*!
     * Symlink if there is one.
     *
     * Returns the symlink, or QString()
     */
    QString symLinkTarget() const;

    /*!
     * Checks whether the entry is a file.
     *
     * Returns true if this entry is a file
     */
    virtual bool isFile() const;

    /*!
     * Checks whether the entry is a directory.
     *
     * Returns true if this entry is a directory
     */
    virtual bool isDirectory() const;

protected:
    KArchive *archive() const;

protected:
    virtual void virtual_hook(int id, void *data);

private:
    KArchiveEntryPrivate *const d;
};

#endif
