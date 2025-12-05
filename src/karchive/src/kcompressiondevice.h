/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef __kcompressiondevice_h
#define __kcompressiondevice_h

#include <karchive_export.h>

#include <QFileDevice>
#include <QIODevice>
#include <QMetaType>
#include <QString>

class KCompressionDevicePrivate;

class KFilterBase;

/*!
 * \class KCompressionDevice
 * \inmodule KArchive
 *
 * A class for reading and writing compressed data onto a device
 * (e.g. file, but other usages are possible, like a buffer or a socket).
 *
 * Use this class to read/write compressed files.
 */

class KARCHIVE_EXPORT KCompressionDevice : public QIODevice // KF7 TODO: consider inheriting from QFileDevice, so apps can use error() generically ?
{
    Q_OBJECT
public:
    /*!
     * \value GZip
     * \value BZip2
     * \value Xz
     * \value None
     * \value[since 5.82] Zstd
     * \value[since 6.15] Lz
     */
    enum CompressionType {
        GZip,
        BZip2,
        Xz,
        None,
        Zstd,
        Lz,
    };

    /*!
     * Constructs a KCompressionDevice for a given CompressionType (e.g. GZip, BZip2 etc.).
     *
     * \a inputDevice input device.
     *
     * \a autoDeleteInputDevice if true, \a inputDevice will be deleted automatically
     *
     * \a type the CompressionType to use.
     */
    KCompressionDevice(QIODevice *inputDevice, bool autoDeleteInputDevice, CompressionType type);

    /*!
     * Constructs a KCompressionDevice for a given CompressionType (e.g. GZip, BZip2 etc.).
     *
     * \a fileName the name of the file to filter.
     *
     * \a type the CompressionType to use.
     */
    KCompressionDevice(const QString &fileName, CompressionType type);

    /*!
     * Constructs a KCompressionDevice for a given \a fileName.
     *
     * \a fileName the name of the file to filter.
     *
     * \since 5.85
     */
    explicit KCompressionDevice(const QString &fileName);

    /*!
     * Constructs a KCompressionDevice for a given CompressionType (e.g. GZip, BZip2 etc.).
     *
     * \a inputDevice input device.
     *
     * \a type the CompressionType to use.
     *
     * \a size the size we know the inputDevice with CompressionType type has. If we know it.
     *
     * \since 6.16
     */
    KCompressionDevice(std::unique_ptr<QIODevice> inputDevice, CompressionType type, std::optional<qint64> size = {});

    /*!
     * Destructs the KCompressionDevice.
     *
     * Calls close() if the filter device is still open.
     */
    ~KCompressionDevice() override;

    /*!
     * The compression actually used by this device.
     *
     * If the support for the compression requested in the constructor
     * is not available, then the device will use None.
     */
    CompressionType compressionType() const;

    [[nodiscard]] bool open(QIODevice::OpenMode mode) override;

    void close() override;

    qint64 size() const override;

    /*!
     * For writing gzip compressed files only:
     * set the name of the original file, to be used in the gzip header.
     *
     * \a fileName the name of the original file
     */
    void setOrigFileName(const QByteArray &fileName);

    /*!
     * Call this let this device skip the gzip headers when reading/writing.
     * This way KCompressionDevice (with gzip filter) can be used as a direct wrapper
     * around zlib - this is used by KZip.
     */
    void setSkipHeaders();

    /*!
     * \reimp
     * That one can be quite slow, when going back. Use with care.
     */
    bool seek(qint64) override;

    bool atEnd() const override;

    /*!
     * Call this to create the appropriate filter for the CompressionType
     * named \a type.
     *
     * \a type the type of the compression filter
     *
     * Returns the filter for the \a type, or 0 if not found
     */
    static KFilterBase *filterForCompressionType(CompressionType type);

    /*!
     * Returns the compression type for the given MIME type, if possible. Otherwise returns None.
     *
     * This handles simple cases like application/gzip, but also application/x-compressed-tar, and inheritance.
     * \since 5.85
     */
    static CompressionType compressionTypeForMimeType(const QString &mimetype);

    /*!
     * Returns the error code from the last failing operation.
     * This is especially useful after calling close(), which unfortunately returns void
     * (see https://bugreports.qt.io/browse/QTBUG-70033), to see if the flushing done by close
     * was able to write all the data to disk.
     */
    QFileDevice::FileError error() const;

protected:
    friend class K7Zip;

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    KFilterBase *filterBase();

private:
    friend KCompressionDevicePrivate;
    KCompressionDevicePrivate *const d;
};

Q_DECLARE_METATYPE(KCompressionDevice::CompressionType)

#endif
