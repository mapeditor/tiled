/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef __kcompressiondevice_h
#define __kcompressiondevice_h

#include <karchive_export.h>
#include <QIODevice>
#include <QFileDevice>
#include <QString>
#include <QMetaType>
class KCompressionDevicePrivate;

class KFilterBase;

/**
 * @class KCompressionDevice kcompressiondevice.h KCompressionDevice
 *
 * A class for reading and writing compressed data onto a device
 * (e.g. file, but other usages are possible, like a buffer or a socket).
 *
 * Use this class to read/write compressed files.
 */

class KARCHIVE_EXPORT KCompressionDevice : public QIODevice // KF6 TODO: consider inheriting from QFileDevice, so apps can use error() generically ?
{
    Q_OBJECT
public:
    enum CompressionType {
        GZip,
        BZip2,
        Xz,
        None
    };

    /**
     * Constructs a KCompressionDevice for a given CompressionType (e.g. GZip, BZip2 etc.).
     * @param inputDevice input device.
     * @param autoDeleteInputDevice if true, @p inputDevice will be deleted automatically
     * @param type the CompressionType to use.
     */
    KCompressionDevice(QIODevice *inputDevice, bool autoDeleteInputDevice, CompressionType type);

    /**
     * Constructs a KCompressionDevice for a given CompressionType (e.g. GZip, BZip2 etc.).
     * @param fileName the name of the file to filter.
     * @param type the CompressionType to use.
     */
    KCompressionDevice(const QString &fileName, CompressionType type);

    /**
     * Destructs the KCompressionDevice.
     * Calls close() if the filter device is still open.
     */
    virtual ~KCompressionDevice();

    /**
     * The compression actually used by this device.
     * If the support for the compression requested in the constructor
     * is not available, then the device will use None.
     */
    CompressionType compressionType() const;

    /**
     * Open for reading or writing.
     */
    bool open(QIODevice::OpenMode mode) override;

    /**
     * Close after reading or writing.
     */
    void close() override;

    /**
     * For writing gzip compressed files only:
     * set the name of the original file, to be used in the gzip header.
     * @param fileName the name of the original file
     */
    void setOrigFileName(const QByteArray &fileName);

    /**
     * Call this let this device skip the gzip headers when reading/writing.
     * This way KCompressionDevice (with gzip filter) can be used as a direct wrapper
     * around zlib - this is used by KZip.
     */
    void setSkipHeaders();

    /**
     * That one can be quite slow, when going back. Use with care.
     */
    bool seek(qint64) override;

    bool atEnd() const override;

    /**
     * Call this to create the appropriate filter for the CompressionType
     * named @p type.
     * @param type the type of the compression filter
     * @return the filter for the @p type, or 0 if not found
     */
    static KFilterBase *filterForCompressionType(CompressionType type);

    /**
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
