/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2015 Luiz Romário Santana Rios <luizromario@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompressiondevicetest.h"
#include "kcompressiondevice_p.h"

#include <config-compression.h>

#include <QBuffer>
#include <QDir>
#include <QDirIterator>
#include <QTemporaryDir>
#include <QTest>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVector>

QTEST_MAIN(KCompressionDeviceTest)

static QString archiveFileName(const QString &extension)
{
    return QFINDTESTDATA(QString("kcompressiondevice_test.%1").arg(extension));
}

QNetworkReply *KCompressionDeviceTest::getArchive(const QString &extension)
{
    const QString kcompressionTest = archiveFileName(extension);
    QNetworkReply *r = qnam.get(QNetworkRequest(QUrl::fromLocalFile(kcompressionTest)));

    QEventLoop l;
    connect(&qnam, &QNetworkAccessManager::finished, &l, &QEventLoop::quit);
    l.exec();

    return r;
}

QString KCompressionDeviceTest::formatExtension(KCompressionDevice::CompressionType type) const
{
    switch (type) {
    case KCompressionDevice::GZip:
        return "tar.gz";
    case KCompressionDevice::BZip2:
        return "tar.bz2";
    case KCompressionDevice::Xz:
        return "tar.xz";
    case KCompressionDevice::None:
        return QString();
    }
    return QString(); // silence compiler warning
}

void KCompressionDeviceTest::setDeviceToArchive(
        QIODevice *d,
        KCompressionDevice::CompressionType type)
{
    KCompressionDevice *devRawPtr = new KCompressionDevice(d, true, type);
    archive.reset(new KTar(devRawPtr));
    device.reset(devRawPtr);
}

void KCompressionDeviceTest::testBufferedDevice(KCompressionDevice::CompressionType type)
{
    QNetworkReply *r = getArchive(formatExtension(type));
    const QByteArray data = r->readAll();
    QVERIFY(!data.isEmpty());
    const int expectedSize = QFileInfo(archiveFileName(formatExtension(type))).size();
    QVERIFY(expectedSize > 0);
    QCOMPARE(data.size(), expectedSize);
    QBuffer *b = new QBuffer;
    b->setData(data);

    setDeviceToArchive(b, type);
    testExtraction();
}

void KCompressionDeviceTest::testExtraction()
{
    QTemporaryDir temp;
    QString oldCurrentDir = QDir::currentPath();
    QDir::setCurrent(temp.path());

    QVERIFY(archive->open(QIODevice::ReadOnly));
    QVERIFY(archive->directory()->copyTo("."));
    QVERIFY(QDir("examples").exists());
    QVERIFY(QDir("examples/bzip2gzip").exists());
    QVERIFY(QDir("examples/helloworld").exists());
    QVERIFY(QDir("examples/tarlocalfiles").exists());
    QVERIFY(QDir("examples/unzipper").exists());

    QVector<QString> fileList;
    fileList
            << QLatin1String("examples/bzip2gzip/CMakeLists.txt")
            << QLatin1String("examples/bzip2gzip/main.cpp")
            << QLatin1String("examples/helloworld/CMakeLists.txt")
            << QLatin1String("examples/helloworld/helloworld.pro")
            << QLatin1String("examples/helloworld/main.cpp")
            << QLatin1String("examples/tarlocalfiles/CMakeLists.txt")
            << QLatin1String("examples/tarlocalfiles/main.cpp")
            << QLatin1String("examples/unzipper/CMakeLists.txt")
            << QLatin1String("examples/unzipper/main.cpp");

    for (const QString& s : qAsConst(fileList)) {
        QFileInfo extractedFile(s);
        QFileInfo sourceFile(QFINDTESTDATA("../" + s));

        QVERIFY(extractedFile.exists());
        QCOMPARE(extractedFile.size(), sourceFile.size());
    }
    QDir::setCurrent(oldCurrentDir);
}

void KCompressionDeviceTest::regularKTarUsage()
{
    archive.reset(new KTar(QFINDTESTDATA("kcompressiondevice_test.tar.gz")));
    device.reset();

    testExtraction();
}

void KCompressionDeviceTest::testGZipBufferedDevice()
{
    testBufferedDevice(KCompressionDevice::GZip);
}

void KCompressionDeviceTest::testBZip2BufferedDevice()
{
#if HAVE_BZIP2_SUPPORT
    testBufferedDevice(KCompressionDevice::BZip2);
#else
    QSKIP("This test needs bzip2 support");
#endif
}

void KCompressionDeviceTest::testXzBufferedDevice()
{
#if HAVE_XZ_SUPPORT
    testBufferedDevice(KCompressionDevice::Xz);
#else
    QSKIP("This test needs xz support");
#endif
}

void KCompressionDeviceTest::testWriteErrorOnOpen()
{
    // GIVEN
    QString fileName("/I/dont/exist/kcompressiondevicetest-write.gz");
    KCompressionDevice dev(fileName, KCompressionDevice::GZip);
    // WHEN
    QVERIFY(!dev.open(QIODevice::WriteOnly));
    // THEN
    QCOMPARE(dev.error(), QFileDevice::OpenError);
#ifdef Q_OS_WIN
    QCOMPARE(dev.errorString(), QStringLiteral("The system cannot find the path specified."));
#else
    QCOMPARE(dev.errorString(), QStringLiteral("No such file or directory"));
#endif
}

void KCompressionDeviceTest::testWriteErrorOnClose()
{
    // GIVEN
    QFile file("kcompressiondevicetest-write.gz");
    KCompressionDevice dev(&file, false, KCompressionDevice::GZip);
    QVERIFY(dev.open(QIODevice::WriteOnly));
    const QByteArray data = "Hello world";
    QCOMPARE(dev.write(data), data.size());
    // This is nasty, it's just a way to try and trigger an error on flush, without filling up a partition first ;)
    file.close();
    QVERIFY(file.open(QIODevice::ReadOnly));
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::write (QFile, \"kcompressiondevicetest-write.gz\"): ReadOnly device");

    // WHEN
    dev.close(); // I want a QVERIFY here... https://bugreports.qt.io/browse/QTBUG-70033

    // THEN
    QCOMPARE(int(dev.error()), int(QFileDevice::WriteError));
}

void KCompressionDeviceTest::testSeekReadUncompressedBuffer_data()
{
    QTest::addColumn<int>("dataSize");
    QTest::addColumn<int>("realDataPos");
    QTest::newRow("1.5buffer") << BUFFER_SIZE + BUFFER_SIZE / 2 << BUFFER_SIZE;
    QTest::newRow("5seekbuffer") << 5 * SEEK_BUFFER_SIZE << 4 * SEEK_BUFFER_SIZE;
}

void KCompressionDeviceTest::testSeekReadUncompressedBuffer()
{
    QFETCH(int, dataSize);
    QFETCH(int, realDataPos);

    QByteArray ba(dataSize, 0);

    // all data is zero except after realDataPos that it's 0 to 9
    for (int i = 0; i < 10; ++i) {
        ba[realDataPos + i] = i;
    }

    QBuffer b;
    b.setData(ba);
    QVERIFY(b.open(QIODevice::ReadOnly));

    KCompressionDevice kcd(&b, false, KCompressionDevice::GZip);
    QVERIFY(kcd.open(QIODevice::ReadOnly));
    QVERIFY(kcd.seek(realDataPos));

    // the 10 bytes after realDataPos should be 0 to 9
    const QByteArray kcdData = kcd.read(10);
    QCOMPARE(kcdData.size(), 10);
    for (int i = 0; i < kcdData.size(); ++i) {
        QCOMPARE(kcdData[i], i);
    }
}
