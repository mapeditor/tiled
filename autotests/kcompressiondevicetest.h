/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2015 Luiz Romário Santana Rios <luizromario@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPRESSIONDEVICETEST_H
#define KCOMPRESSIONDEVICETEST_H

#include <QObject>

#include <QNetworkAccessManager>
#include <QScopedPointer>

#include <KTar>
#include <KCompressionDevice>

class QNetworkReply;

class KCompressionDeviceTest : public QObject
{
    Q_OBJECT

private:
    QNetworkReply *getArchive(const QString &extension);
    QString formatExtension(KCompressionDevice::CompressionType type) const;

    void setDeviceToArchive(
            QIODevice *d,
            KCompressionDevice::CompressionType type);

    void testBufferedDevice(KCompressionDevice::CompressionType type);
    void testExtraction();

    QNetworkAccessManager qnam;
    QScopedPointer<KCompressionDevice> device;
    QScopedPointer<KTar> archive;

private Q_SLOTS:
    void regularKTarUsage();
    void testGZipBufferedDevice();
    void testBZip2BufferedDevice();
    void testXzBufferedDevice();

    void testWriteErrorOnOpen();
    void testWriteErrorOnClose();

    void testSeekReadUncompressedBuffer_data();
    void testSeekReadUncompressedBuffer();
};

#endif
