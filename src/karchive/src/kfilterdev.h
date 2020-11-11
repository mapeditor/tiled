/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef __kfilterdev_h
#define __kfilterdev_h

#include <karchive_export.h>
#include <kcompressiondevice.h>
#include <QString>

class QFile;
class KFilterBase;

/**
 * @class KFilterDev kfilterdev.h KFilterDev
 *
 * A class for reading and writing compressed data onto a device
 * (e.g. file, but other usages are possible, like a buffer or a socket).
 *
 * To simply read/write compressed files, see deviceForFile.
 *
 * KFilterDev adds MIME type support to KCompressionDevice, and also
 * provides compatibility methods for KDE 4 code.
 *
 * @author David Faure <faure@kde.org>
 */
class KARCHIVE_EXPORT KFilterDev : public KCompressionDevice
{
    Q_OBJECT
public:
    /**
     * @since 5.0
     * Constructs a KFilterDev for a given FileName.
     * @param fileName the name of the file to filter.
     */
    KFilterDev(const QString &fileName);

    /**
     * Returns the compression type for the given mimetype, if possible. Otherwise returns None.
     * This handles simple cases like application/x-gzip, but also application/x-compressed-tar, and inheritance.
     */
    static CompressionType compressionTypeForMimeType(const QString &mimetype);

#if KARCHIVE_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * @deprecated Since 5.0, use the constructor instead (if mimetype is empty), or KCompressionDevice (if
     * the mimetype is known).
     *
     * Use:
     * KFilterDev dev(fileName)
     * instead of:
     * QIODevice * dev = KFilterDev::deviceForFile(fileName);
     *
     * If the mimetype was specified explicitly, use:
     * KCompressionDevice dev(fileName, KCompressionDevice::GZip);
     * instead of:
     * QIODevice * dev = KFilterDev::deviceForFile(fileName, "application/gzip");
     *
     * Creates an i/o device that is able to read from @p fileName,
     * whether it's compressed or not. Available compression filters
     * (gzip/bzip2 etc.) will automatically be used.
     *
     * The compression filter to be used is determined from the @p fileName
     * if @p mimetype is empty. Pass "application/x-gzip" or "application/x-bzip"
     * to force the corresponding decompression filter, if available.
     *
     * Warning: application/x-bzip may not be available.
     * In that case a QFile opened on the compressed data will be returned !
     * Use KFilterBase::findFilterByMimeType and code similar to what
     * deviceForFile is doing, to better control what's happening.
     *
     * The returned QIODevice has to be deleted after using.
     *
     * @param fileName the name of the file to filter
     * @param mimetype the mime type of the file to filter, or QString() if unknown
     * @param forceFilter if true, the function will either find a compression filter, or return 0.
     *                    If false, it will always return a QIODevice. If no
     *                    filter is available it will return a simple QFile.
     *                    This can be useful if the file is usable without a filter.
     * @return if a filter has been found, the KCompressionDevice for the filter. If the
     *         filter does not exist, the return value depends on @p forceFilter.
     *         The returned KCompressionDevice has to be deleted after using.
     */
    KARCHIVE_DEPRECATED_VERSION(5, 0, "See API docs")
    static KCompressionDevice *deviceForFile(const QString &fileName,
            const QString &mimetype = QString(),
            bool forceFilter = false)
    {
        KCompressionDevice *device;
        if (mimetype.isEmpty()) {
            device = new KFilterDev(fileName);
        } else {
            device = new KCompressionDevice(fileName, compressionTypeForMimeType(mimetype));
        }
        if (device->compressionType() == KCompressionDevice::None && forceFilter) {
            delete device;
            return nullptr;
        } else {
            return device;
        }
    }
#endif

#if KARCHIVE_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * @deprecated Since 5.0, use KCompressionDevice
     *
     * Use:
     * KCompressionDevice::CompressionType type = KFilterDev::compressionTypeForMimeType(mimeType);
     * KCompressionDevice flt(&file, false, type);
     * instead of:
     * QIODevice *flt = KFilterDev::device(&file, mimeType, false);
     *
     * Creates an i/o device that is able to read from the QIODevice @p inDevice,
     * whether the data is compressed or not. Available compression filters
     * (gzip/bzip2 etc.) will automatically be used.
     *
     * The compression filter to be used is determined @p mimetype .
     * Pass "application/x-gzip" or "application/x-bzip"
     * to use the corresponding decompression filter.
     *
     * Warning: application/x-bzip may not be available.
     * In that case 0 will be returned !
     *
     * The returned QIODevice has to be deleted after using.
     * @param inDevice input device. Won't be deleted if @p autoDeleteInDevice = false
     * @param mimetype the mime type for the filter
     * @param autoDeleteInDevice if true, @p inDevice will be deleted automatically
     * @return a KCompressionDevice that filters the original stream. Must be deleted after using
     */
    KARCHIVE_DEPRECATED_VERSION(5, 0, "See API docs")
    static KCompressionDevice *device(QIODevice *inDevice, const QString &mimetype,
            bool autoDeleteInDevice = true)
    {
        if (inDevice == nullptr) {
            return nullptr;
        }
        CompressionType type = compressionTypeForMimeType(mimetype);
        KCompressionDevice *device = new KCompressionDevice(inDevice, autoDeleteInDevice, type);
        return device;
    }
#endif
};

#endif
