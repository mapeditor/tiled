/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>

   Moved from ktar.h by Roberto Teixeira <maragato@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KARCHIVE_H
#define KARCHIVE_H

#include <sys/stat.h>
#include <sys/types.h>

#include <QCoreApplication>
#include <QDate>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QIODevice>

#include <karchive_export.h>

#ifdef Q_OS_WIN
#include <qplatformdefs.h> // mode_t
#endif

class KArchiveDirectory;
class KArchiveFile;

class KArchivePrivate;
/**
 * @class KArchive karchive.h KArchive
 *
 * KArchive is a base class for reading and writing archives.
 * @short generic class for reading/writing archives
 * @author David Faure <faure@kde.org>
 */
class KARCHIVE_EXPORT KArchive
{
    Q_DECLARE_TR_FUNCTIONS(KArchive)

protected:
    /**
     * Base constructor (protected since this is a pure virtual class).
     * @param fileName is a local path (e.g. "/tmp/myfile.ext"),
     * from which the archive will be read from, or into which the archive
     * will be written, depending on the mode given to open().
     */
    KArchive(const QString &fileName);

    /**
     * Base constructor (protected since this is a pure virtual class).
     * @param dev the I/O device where the archive reads its data
     * Note that this can be a file, but also a data buffer, a compression filter, etc.
     * For a file in writing mode it is better to use the other constructor
     * though, to benefit from the use of QSaveFile when saving.
     */
    KArchive(QIODevice *dev);

public:
    virtual ~KArchive();

    /**
     * Opens the archive for reading or writing.
     * Inherited classes might want to reimplement openArchive instead.
     * @param mode may be QIODevice::ReadOnly or QIODevice::WriteOnly
     * @see close
     */
    virtual bool open(QIODevice::OpenMode mode);

    /**
     * Closes the archive.
     * Inherited classes might want to reimplement closeArchive instead.
     *
     * @return true if close succeeded without problems
     * @see open
     */
    virtual bool close();

    /**
     * Returns a description of the last error
     * @since 5.29
     */
    QString errorString() const;

    /**
     * Checks whether the archive is open.
     * @return true if the archive is opened
     */
    bool isOpen() const;

    /**
     * Returns the mode in which the archive was opened
     * @return the mode in which the archive was opened (QIODevice::ReadOnly or QIODevice::WriteOnly)
     * @see open()
     */
    QIODevice::OpenMode mode() const;

    /**
     * The underlying device.
     * @return the underlying device.
     */
    QIODevice *device() const;

    /**
     * The name of the archive file, as passed to the constructor that takes a
     * fileName, or an empty string if you used the QIODevice constructor.
     * @return the name of the file, or QString() if unknown
     */
    QString fileName() const;

    /**
     * If an archive is opened for reading, then the contents
     * of the archive can be accessed via this function.
     * @return the directory of the archive
     */
    const KArchiveDirectory *directory() const;

    /**
     * Writes a local file into the archive. The main difference with writeFile,
     * is that this method minimizes memory usage, by not loading the whole file
     * into memory in one go.
     *
     * If @p fileName is a symbolic link, it will be written as is, i. e.
     * it will not be resolved before.
     * @param fileName full path to an existing local file, to be added to the archive.
     * @param destName the resulting name (or relative path) of the file in the archive.
     */
    bool addLocalFile(const QString &fileName, const QString &destName);

    /**
     * Writes a local directory into the archive, including all its contents, recursively.
     * Calls addLocalFile for each file to be added.
     *
     * It will also add a @p path that is a symbolic link to a
     * directory. The symbolic link will be dereferenced and the content of the
     * directory it is pointing to added recursively. However, symbolic links
     * *under* @p path will be stored as is.
     * @param path full path to an existing local directory, to be added to the archive.
     * @param destName the resulting name (or relative path) of the file in the archive.
     */
    bool addLocalDirectory(const QString &path, const QString &destName);

    /**
     * If an archive is opened for writing then you can add new directories
     * using this function. KArchive won't write one directory twice.
     *
     * This method also allows some file metadata to be set.
     * However, depending on the archive type not all metadata might be regarded.
     *
     * @param name the name of the directory
     * @param user the user that owns the directory
     * @param group the group that owns the directory
     * @param perm permissions of the directory
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime time of last status change
     */
    bool writeDir(const QString &name, const QString &user = QString(), const QString &group = QString(),
                  mode_t perm = 040755, const QDateTime &atime = QDateTime(),
                  const QDateTime &mtime = QDateTime(), const QDateTime &ctime = QDateTime());

    /**
     * Writes a symbolic link to the archive if supported.
     * The archive must be opened for writing.
     *
     * @param name name of symbolic link
     * @param target target of symbolic link
     * @param user the user that owns the directory
     * @param group the group that owns the directory
     * @param perm permissions of the directory
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime time of last status change
     */
    bool writeSymLink(const QString &name, const QString &target,
                      const QString &user = QString(), const QString &group = QString(),
                      mode_t perm = 0120755, const QDateTime &atime = QDateTime(),
                      const QDateTime &mtime = QDateTime(), const QDateTime &ctime = QDateTime());

#if KARCHIVE_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * @deprecated since 5.0, use writeFile(const QString&,const QByteArray&,mode_t,const QString&,const QString&,const QDateTime&,const QDateTime&,const QDateTime&)
     */
    KARCHIVE_DEPRECATED_VERSION(5, 0, "Use KArchive::writeFile(const QString&,const QByteArray&,mode_t,const QString&,const QString&,const QDateTime&,const QDateTime&,const QDateTime&)")
    bool writeFile(const QString &name, const QString &user, const QString &group,
                   const char *data, qint64 size,
                   mode_t perm = 0100644, const QDateTime &atime = QDateTime(),
                   const QDateTime &mtime = QDateTime(), const QDateTime &ctime = QDateTime())
    {
        QByteArray array(data, size);
        return writeFile(name, array, perm, user, group, atime, mtime, ctime);
    }
    // The above can lead to ambiguous calls when using "..." for the first 4 arguments,
    // but that's good, better than unexpected behavior due to the signature change.
#endif

    /**
     * Writes a new file into the archive.
     *
     * The archive must be opened for writing first.
     *
     * The necessary parent directories are created automatically
     * if needed. For instance, writing "mydir/test1" does not
     * require creating the directory "mydir" first.
     *
     * This method also allows some file metadata to be
     * set. However, depending on the archive type not all metadata might be
     * written out.
     *
     * @param name the name of the file
     * @param data the data to write
     * @param perm permissions of the file
     * @param user the user that owns the file
     * @param group the group that owns the file
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime time of last status change
     */
    bool writeFile(const QString &name, const QByteArray &data,
                   mode_t perm = 0100644,
                   const QString &user = QString(), const QString &group = QString(),
                   const QDateTime &atime = QDateTime(),
                   const QDateTime &mtime = QDateTime(), const QDateTime &ctime = QDateTime());

    /**
     * Here's another way of writing a file into an archive:
     * Call prepareWriting(), then call writeData()
     * as many times as wanted then call finishWriting( totalSize ).
     * For tar.gz files, you need to know the size before hand, it is needed in the header!
     * For zip files, size isn't used.
     *
     * This method also allows some file metadata to be
     * set. However, depending on the archive type not all metadata might be
     * regarded.
     * @param name the name of the file
     * @param user the user that owns the file
     * @param group the group that owns the file
     * @param size the size of the file
     * @param perm permissions of the file
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime time of last status change
     */
    bool prepareWriting(const QString &name, const QString &user,
                        const QString &group, qint64 size,
                        mode_t perm = 0100644, const QDateTime &atime = QDateTime(),
                        const QDateTime &mtime = QDateTime(), const QDateTime &ctime = QDateTime());

    /**
     * Write data into the current file - to be called after calling prepareWriting
     */
    virtual bool writeData(const char *data, qint64 size);

    /**
     * Call finishWriting after writing the data.
     * @param size the size of the file
     * @see prepareWriting()
     */
    bool finishWriting(qint64 size);

protected:
    /**
     * Opens an archive for reading or writing.
     * Called by open.
     * @param mode may be QIODevice::ReadOnly or QIODevice::WriteOnly
     */
    virtual bool openArchive(QIODevice::OpenMode mode) = 0;

    /**
     * Closes the archive.
     * Called by close.
     */
    virtual bool closeArchive() = 0;

    /**
     * Sets error description
     * @param errorStr error description
     * @since 5.29
     */
    void setErrorString(const QString &errorStr);

    /**
     * Retrieves or create the root directory.
     * The default implementation assumes that openArchive() did the parsing,
     * so it creates a dummy rootdir if none was set (write mode, or no '/' in the archive).
     * Reimplement this to provide parsing/listing on demand.
     * @return the root directory
     */
    virtual KArchiveDirectory *rootDir();

    /**
     * Write a directory to the archive.
     * This virtual method must be implemented by subclasses.
     *
     * Depending on the archive type not all metadata might be used.
     *
     * @param name the name of the directory
     * @param user the user that owns the directory
     * @param group the group that owns the directory
     * @param perm permissions of the directory. Use 040755 if you don't have any other information.
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime time of last status change
     * @see writeDir
     */
    virtual bool doWriteDir(const QString &name, const QString &user, const QString &group,
                            mode_t perm, const QDateTime &atime, const QDateTime &mtime, const QDateTime &ctime) = 0;

    /**
     * Writes a symbolic link to the archive.
     * This virtual method must be implemented by subclasses.
     *
     * @param name name of symbolic link
     * @param target target of symbolic link
     * @param user the user that owns the directory
     * @param group the group that owns the directory
     * @param perm permissions of the directory
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime time of last status change
     * @see writeSymLink
     */
    virtual bool doWriteSymLink(const QString &name, const QString &target,
                                const QString &user, const QString &group,
                                mode_t perm, const QDateTime &atime, const QDateTime &mtime, const QDateTime &ctime) = 0;

    /**
     * This virtual method must be implemented by subclasses.
     *
     * Depending on the archive type not all metadata might be used.
     *
     * @param name the name of the file
     * @param user the user that owns the file
     * @param group the group that owns the file
     * @param size the size of the file
     * @param perm permissions of the file. Use 0100644 if you don't have any more specific permissions to set.
     * @param atime time the file was last accessed
     * @param mtime modification time of the file
     * @param ctime time of last status change
     * @see prepareWriting
     */
    virtual bool doPrepareWriting(const QString &name, const QString &user,
                                  const QString &group, qint64 size, mode_t perm,
                                  const QDateTime &atime, const QDateTime &mtime, const QDateTime &ctime) = 0;

    /**
     * Called after writing the data.
     * This virtual method must be implemented by subclasses.
     *
     * @param size the size of the file
     * @see finishWriting()
     */
    virtual bool doFinishWriting(qint64 size) = 0;

    /**
     * Ensures that @p path exists, create otherwise.
     * This handles e.g. tar files missing directory entries, like mico-2.3.0.tar.gz :)
     * @param path the path of the directory
     * @return the directory with the given @p path
     */
    KArchiveDirectory *findOrCreate(const QString &path);

    /**
     * Can be reimplemented in order to change the creation of the device
     * (when using the fileName constructor). By default this method uses
     * QSaveFile when saving, and a simple QFile on reading.
     * This method is called by open().
     */
    virtual bool createDevice(QIODevice::OpenMode mode);

    /**
     * Can be called by derived classes in order to set the underlying device.
     * Note that KArchive will -not- own the device, it must be deleted by the derived class.
     */
    void setDevice(QIODevice *dev);

    /**
     * Derived classes call setRootDir from openArchive,
     * to set the root directory after parsing an existing archive.
     */
    void setRootDir(KArchiveDirectory *rootDir);

protected:
    virtual void virtual_hook(int id, void *data);
private:
    friend class KArchivePrivate;
    KArchivePrivate *const d;
};

// for source compat
#include "karchivefile.h"
#include "karchivedirectory.h"

#endif
