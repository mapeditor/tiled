/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KTAR_H
#define KTAR_H

#include <karchive.h>

/*!
 * \class KTar
 * \inmodule KArchive
 *
 * \brief A class for reading / writing (optionally compressed) tar archives.
 *
 * KTar allows you to read and write tar archives, including those
 * that are compressed using gzip, bzip2, xz or lzip.
 */
class KARCHIVE_EXPORT KTar : public KArchive
{
    Q_DECLARE_TR_FUNCTIONS(KTar)

public:
    /*!
     * Creates an instance that operates on the given filename
     * using the compression filter associated to given mimetype.
     *
     * \a filename is a local path (e.g. "/home/weis/myfile.tgz")
     *
     * \a mimetype "application/gzip" (before 5.85: "application/x-gzip"), "application/x-bzip",
     * "application/x-xz", "application/zstd" (since 5.82)
     * "application/x-lzip" (since 6.15)
     * Do not use application/x-compressed-tar or similar - you only need to
     * specify the compression layer !  If the mimetype is omitted, it
     * will be determined from the filename.
     */
    explicit KTar(const QString &filename, const QString &mimetype = QString());

    /*!
     * Creates an instance that operates on the given device.
     *
     * The device can be compressed (KCompressionDevice) or not (QFile, etc.).
     *
     * \warning Do not assume that giving a QFile here will decompress the file,
     * in case it's compressed!
     *
     * \a dev the device to read from. If the source is compressed, the
     * QIODevice must take care of decompression
     */
    explicit KTar(QIODevice *dev);

    /*!
     * If the tar ball is still opened, then it will be
     * closed automatically by the destructor.
     */
    ~KTar() override;

    /*!
     * Special function for setting the "original file name" in the gzip header,
     * when writing a tar.gz file. It appears when using in the "file" command,
     * for instance. Should only be called if the underlying device is a KCompressionDevice!
     *
     * \a fileName the original file name
     */
    void setOrigFileName(const QByteArray &fileName);

protected:
    /// Reimplemented from KArchive
    bool doWriteSymLink(const QString &name,
                        const QString &target,
                        const QString &user,
                        const QString &group,
                        mode_t perm,
                        const QDateTime &atime,
                        const QDateTime &mtime,
                        const QDateTime &ctime) override;
    /// Reimplemented from KArchive
    bool doWriteDir(const QString &name,
                    const QString &user,
                    const QString &group,
                    mode_t perm,
                    const QDateTime &atime,
                    const QDateTime &mtime,
                    const QDateTime &ctime) override;
    /// Reimplemented from KArchive
    bool doPrepareWriting(const QString &name,
                          const QString &user,
                          const QString &group,
                          qint64 size,
                          mode_t perm,
                          const QDateTime &atime,
                          const QDateTime &mtime,
                          const QDateTime &ctime) override;
    /// Reimplemented from KArchive
    bool doFinishWriting(qint64 size) override;

    /*
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     * \a mode the mode of the file
     */
    bool openArchive(QIODevice::OpenMode mode) override;
    bool closeArchive() override;

    bool createDevice(QIODevice::OpenMode mode) override;

private:
protected:
    void virtual_hook(int id, void *data) override;

private:
    class KTarPrivate;
    KTarPrivate *const d;
};

#endif
