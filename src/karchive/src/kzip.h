/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2002 Holger Schroeder <holger-kde@holgis.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KZIP_H
#define KZIP_H

#include <karchive.h>

#include "kzipfileentry.h" // for source compat

class KZipFileEntry;
/*!
 * \class KZip
 * \inmodule KArchive
 *
 * \brief A class for reading / writing zip archives.
 *
 * You can use it in QIODevice::ReadOnly or in QIODevice::WriteOnly mode, and it
 * behaves just as expected.
 * It can also be used in QIODevice::ReadWrite mode, in this case one can
 * append files to an existing zip archive. When you append new files, which
 * are not yet in the zip, it works as expected, i.e. the files are appended at the end.
 * When you append a file, which is already in the file, the reference to the
 * old file is dropped and the new one is added to the zip - but the
 * old data from the file itself is not deleted, it is still in the
 * zipfile. So when you want to have a small and garbage-free zipfile,
 * just read the contents of the appended zip file and write it to a new one
 * in QIODevice::WriteOnly mode. This is especially important when you don't want
 * to leak information of how intermediate versions of files in the zip
 * were looking.
 *
 * For more information on the zip fileformat go to
 * http://www.pkware.com/products/enterprise/white_papers/appnote.html
 */
class KARCHIVE_EXPORT KZip : public KArchive
{
    Q_DECLARE_TR_FUNCTIONS(KZip)

public:
    /*!
     * Creates an instance that operates on the given filename.
     * using the compression filter associated to given mimetype.
     *
     * \a filename is a local path (e.g. "/home/holger/myfile.zip")
     */
    explicit KZip(const QString &filename);

    /*!
     * Creates an instance that operates on the given device.
     *
     * The device can be compressed (KCompressionDevice) or not (QFile, etc.).
     *
     * \a dev the device to access
     *
     * \warning Do not assume that giving a QFile here will decompress the file,
     * in case it's compressed!
     */
    explicit KZip(QIODevice *dev);

    /*!
     * If the zip file is still opened, then it will be
     * closed automatically by the destructor.
     */
    ~KZip() override;

    /*!
     * Describes the contents of the "extra field" for a given file in the Zip archive.
     *
     * \value NoExtraField No extra field
     * \value ModificationTime Modification time ("extended timestamp" header)
     * \omitvalue DefaultExtraField
     */
    enum ExtraField {
        NoExtraField = 0,
        ModificationTime = 1,
        DefaultExtraField = 1,
    };

    /*!
     * Call this before writeFile or prepareWriting, to define what the next
     * file to be written should have in its extra field.
     *
     * \a ef the type of "extra field"
     * \sa extraField()
     */
    void setExtraField(ExtraField ef);

    /*!
     * The current type of "extra field" that will be used for new files.
     *
     * Returns the current type of "extra field"
     * \sa setExtraField()
     */
    ExtraField extraField() const;

    /*!
     * Describes the compression type for a given file in the Zip archive.
     *
     * \value NoCompression Uncompressed
     * \value DeflateCompression Deflate compression method
     */
    enum Compression {
        NoCompression = 0,
        DeflateCompression = 1,
    };

    /*!
     * Call this before writeFile or prepareWriting, to define whether the next
     * files to be written should be compressed or not.
     *
     * \a c the new compression mode
     * \sa compression()
     */
    void setCompression(Compression c);

    /*!
     * The current compression mode that will be used for new files.
     *
     * Returns the current compression mode
     * \sa setCompression()
     */
    Compression compression() const;

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
    bool doPrepareWriting(const QString &name,
                          const QString &user,
                          const QString &group,
                          qint64 size,
                          mode_t perm,
                          const QDateTime &atime,
                          const QDateTime &mtime,
                          const QDateTime &creationTime) override;

    /*
     * Write data to a file that has been created using prepareWriting().
     * \a size the size of the file
     * Returns true if successful, false otherwise
     */
    bool doFinishWriting(qint64 size) override;

    /*
     * Write data to a file that has been created using prepareWriting().
     * \a data a pointer to the data
     * \a size the size of the chunk
     * Returns true if successful, false otherwise
     */
    bool doWriteData(const char *data, qint64 size) override;

    /*
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     * \a mode the mode of the file
     */
    bool openArchive(QIODevice::OpenMode mode) override;

    /// Closes the archive
    bool closeArchive() override;

    /// Reimplemented from KArchive
    bool doWriteDir(const QString &name,
                    const QString &user,
                    const QString &group,
                    mode_t perm,
                    const QDateTime &atime,
                    const QDateTime &mtime,
                    const QDateTime &ctime) override;

protected:
    void virtual_hook(int id, void *data) override;

private:
    class KZipPrivate;
    KZipPrivate *const d;
};

#endif
