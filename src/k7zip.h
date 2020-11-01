/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef K7ZIP_H
#define K7ZIP_H

#include <karchive.h>

/**
 * @class K7Zip k7zip.h K7Zip
 *
 * A class for reading / writing p7zip archives.
 *
 * @author Mario Bensi
 */
class KARCHIVE_EXPORT K7Zip : public KArchive
{
    Q_DECLARE_TR_FUNCTIONS(K7Zip)

public:
    /**
     * Creates an instance that operates on the given filename
     * using the compression filter associated to given mimetype.
     *
     * @param filename is a local path (e.g. "/home/user/myfile.7z")
     */
    explicit K7Zip(const QString &filename);

    /**
     * Creates an instance that operates on the given device.
     * The device can be compressed (KFilterDev) or not (QFile, etc.).
     * @warning Do not assume that giving a QFile here will decompress the file,
     * in case it's compressed!
     * @param dev the device to read from. If the source is compressed, the
     * QIODevice must take care of decompression
     */
    explicit K7Zip(QIODevice *dev);

    /**
     * If the archive is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~K7Zip();

protected:

    /// Reimplemented from KArchive
    bool doWriteSymLink(const QString &name, const QString &target,
                        const QString &user, const QString &group,
                        mode_t perm, const QDateTime &atime, const QDateTime &mtime, const QDateTime &ctime) override;
    /// Reimplemented from KArchive
    bool doWriteDir(const QString &name, const QString &user, const QString &group,
                    mode_t perm, const QDateTime &atime, const QDateTime &mtime, const QDateTime &ctime) override;
    /// Reimplemented from KArchive
    bool doPrepareWriting(const QString &name, const QString &user,
                          const QString &group, qint64 size, mode_t perm,
                          const QDateTime &atime, const QDateTime &mtime, const QDateTime &ctime) override;
    /// Reimplemented from KArchive
    bool doFinishWriting(qint64 size) override;

    /// Reimplemented from KArchive
    bool writeData(const char *data, qint64 size) override;

    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     * @param mode the mode of the file
     */
    bool openArchive(QIODevice::OpenMode mode) override;
    bool closeArchive() override;

protected:
    void virtual_hook(int id, void *data) override;
private:
    class K7ZipPrivate;
    K7ZipPrivate *const d;
};

#endif
