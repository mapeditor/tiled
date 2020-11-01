/*
 *  SPDX-FileCopyrightText: 2002-2005 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kfiltertest.h"

#include <QTest>
#include <QBuffer>
#include <QTextCodec>
#include <QSaveFile>
#include <QRandomGenerator>

#include <config-compression.h>
#include "kfilterdev.h"
#include "kfilterbase.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <zlib.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <limits.h>
#endif

QTEST_MAIN(KFilterTest)

void KFilterTest::initTestCase()
{
    qRegisterMetaType<KCompressionDevice::CompressionType>();
    const QString currentdir = QDir::currentPath();
    pathgz = currentdir + "/test.gz";
    pathbz2 = currentdir + "/test.bz2";
    pathxz = currentdir + "/test.xz";
    pathnone = currentdir + "/test.txt";

    // warning, update the COMPAREs in test_block_write() if changing the test data...
    testData = "hello world\n";
}

void KFilterTest::test_block_write(const QString &fileName, const QByteArray &data)
{
    KFilterDev dev(fileName);
    bool ok = dev.open(QIODevice::WriteOnly);
    QVERIFY(ok);

    const int ret = dev.write(data);
    QCOMPARE(ret, data.size());

    dev.close();

    QVERIFY(QFile::exists(fileName));
}

void KFilterTest::test_block_write()
{
    qDebug() << " -- test_block_write gzip -- ";
    test_block_write(pathgz, testData);
    QCOMPARE(QFileInfo(pathgz).size(), 33LL);     // size of test.gz

#if HAVE_BZIP2_SUPPORT
    qDebug() << " -- test_block_write bzip2 -- ";
    test_block_write(pathbz2, testData);
    QCOMPARE(QFileInfo(pathbz2).size(), 52LL);     // size of test.bz2
#endif

#if HAVE_XZ_SUPPORT
    qDebug() << " -- test_block_write xz -- ";
    test_block_write(pathxz, testData);
    QCOMPARE(QFileInfo(pathxz).size(), 64LL);     // size of test.lzma
#endif

    qDebug() << " -- test_block_write none -- ";
    test_block_write(pathnone, testData);
    QCOMPARE(QFileInfo(pathnone).size(), 12LL);     // size of test.txt
}

void KFilterTest::test_biggerWrites()
{
    const QString currentdir = QDir::currentPath();
    const QString outFile = currentdir + "/test_big.gz";
    // Find the out-of-bounds from #157706/#188415
    QByteArray data;
    data.reserve(10000);
    auto *generator = QRandomGenerator::global();
    // Prepare test data
    for (int i = 0; i < 8170; ++i) {
        data.append((char)(generator->bounded(256)));
    }
    QCOMPARE(data.size(), 8170);
    // 8170 random bytes compress to 8194 bytes due to the gzip header/footer.
    // Now we can go one by one until we pass 8192.
    // On 32 bit systems it crashed with data.size()=8173, before the "no room for footer yet" fix.
    int compressedSize = 0;
    while (compressedSize < 8200) {
        test_block_write(outFile, data);
        compressedSize = QFileInfo(outFile).size();
        qDebug() << data.size() << "compressed into" << compressedSize;
        // Test data is valid
        test_readall(outFile, QString::fromLatin1("application/x-gzip"), data);

        data.append((char)(generator->bounded(256)));
    }
}

void KFilterTest::test_block_read(const QString &fileName)
{
    KFilterDev dev(fileName);
    bool ok = dev.open(QIODevice::ReadOnly);
    QVERIFY(ok);

    QByteArray array(1024, '\0');
    QByteArray read;
    int n;
    while ((n = dev.read(array.data(), array.size()))) {
        QVERIFY(n > 0);
        read += QByteArray(array.constData(), n);
        //qDebug() << "read returned " << n;
        //qDebug() << "read='" << read << "'";

        // pos() has no real meaning on sequential devices
        // Ah, but kzip uses kfilterdev as a non-sequential device...

        QCOMPARE((int)dev.pos(), (int)read.size());
        //qDebug() << "dev.at = " << dev->at();
    }
    QCOMPARE(read, testData);

    // Test seeking back
    ok = dev.seek(0);
    // test readAll
    read = dev.readAll();
    QCOMPARE(read.size(), testData.size());
    QCOMPARE(read, testData);

    dev.close();
}

void KFilterTest::test_block_read()
{
    qDebug() << " -- test_block_read gzip -- ";
    test_block_read(pathgz);
#if HAVE_BZIP2_SUPPORT
    qDebug() << " -- test_block_read bzip2 -- ";
    test_block_read(pathbz2);
#endif
#if HAVE_XZ_SUPPORT
    qDebug() << " -- test_block_read lzma -- ";
    test_block_read(pathxz);
#endif
    qDebug() << " -- test_block_read none -- ";
    test_block_read(pathnone);
}

void KFilterTest::test_getch(const QString &fileName)
{
    KFilterDev dev(fileName);
    bool ok = dev.open(QIODevice::ReadOnly);
    QVERIFY(ok);
    QByteArray read;
    char ch;
    while (dev.getChar(&ch)) {
        //printf("%c",ch);
        read += ch;
    }
    dev.close();
    QCOMPARE(read, testData);
}

void KFilterTest::test_getch()
{
    qDebug() << " -- test_getch gzip -- ";
    test_getch(pathgz);
#if HAVE_BZIP2_SUPPORT
    qDebug() << " -- test_getch bzip2 -- ";
    test_getch(pathbz2);
#endif
#if HAVE_XZ_SUPPORT
    qDebug() << " -- test_getch lzma -- ";
    test_getch(pathxz);
#endif
    qDebug() << " -- test_getch none -- ";
    test_getch(pathnone);
}

void KFilterTest::test_textstream(const QString &fileName)
{
    KFilterDev dev(fileName);
    bool ok = dev.open(QIODevice::ReadOnly);
    QVERIFY(ok);
    QTextStream ts(&dev);
    QString readStr = ts.readAll();
    dev.close();

    QByteArray read = readStr.toLatin1();
    QCOMPARE(read, testData);
}

void KFilterTest::test_textstream()
{
    qDebug() << " -- test_textstream gzip -- ";
    test_textstream(pathgz);
#if HAVE_BZIP2_SUPPORT
    qDebug() << " -- test_textstream bzip2 -- ";
    test_textstream(pathbz2);
#endif
#if HAVE_XZ_SUPPORT
    qDebug() << " -- test_textstream lzma -- ";
    test_textstream(pathxz);
#endif
    qDebug() << " -- test_textstream none -- ";
    test_textstream(pathnone);
}

void KFilterTest::test_readall(const QString &fileName, const QString &mimeType, const QByteArray &expectedData)
{
    QFile file(fileName);
    KCompressionDevice::CompressionType type = KFilterDev::compressionTypeForMimeType(mimeType);
    KCompressionDevice flt(&file, false, type);
    bool ok = flt.open(QIODevice::ReadOnly);
    QVERIFY(ok);
    const QByteArray read = flt.readAll();
    QCOMPARE(read.size(), expectedData.size());
    QCOMPARE(read, expectedData);

    // Now using QBuffer
    file.seek(0);
    QByteArray compressedData = file.readAll();
    QVERIFY(!compressedData.isEmpty());
    QBuffer buffer(&compressedData);
    KCompressionDevice device(&buffer, false, type);
    QVERIFY(device.open(QIODevice::ReadOnly));
    QCOMPARE(device.readAll(), expectedData);
}

void KFilterTest::test_readall()
{
    qDebug() << " -- test_readall gzip -- ";
    test_readall(pathgz, QString::fromLatin1("application/x-gzip"), testData);
#if HAVE_BZIP2_SUPPORT
    qDebug() << " -- test_readall bzip2 -- ";
    test_readall(pathbz2, QString::fromLatin1("application/x-bzip"), testData);
#endif
#if HAVE_XZ_SUPPORT
    qDebug() << " -- test_readall lzma -- ";
    test_readall(pathxz, QString::fromLatin1("application/x-xz"), testData);
#endif
    qDebug() << " -- test_readall gzip-derived -- ";
    test_readall(pathgz, QString::fromLatin1("image/svg+xml-compressed"), testData);

    qDebug() << " -- test_readall none -- ";
    test_readall(pathnone, QString::fromLatin1("text/plain"), testData);
}

void KFilterTest::test_uncompressed()
{
    // Can KFilterDev handle uncompressed data even when using gzip decompression?
    qDebug() << " -- test_uncompressed -- ";
    QBuffer buffer(&testData);
    buffer.open(QIODevice::ReadOnly);
    KCompressionDevice::CompressionType type = KFilterDev::compressionTypeForMimeType(QString::fromLatin1("application/x-gzip"));
    KCompressionDevice flt(&buffer, false, type);
    bool ok = flt.open(QIODevice::ReadOnly);
    QVERIFY(ok);
    QByteArray read = flt.readAll();
    QCOMPARE(read.size(), testData.size());
    QCOMPARE(read, testData);
}

void KFilterTest::test_findFilterByMimeType_data()
{
    QTest::addColumn<QString>("mimeType");
    QTest::addColumn<KCompressionDevice::CompressionType>("type");

    // direct mimetype name
    QTest::newRow("application/x-gzip") << QString::fromLatin1("application/x-gzip") << KCompressionDevice::GZip;
#if HAVE_BZIP2_SUPPORT
    QTest::newRow("application/x-bzip") << QString::fromLatin1("application/x-bzip") << KCompressionDevice::BZip2;
    QTest::newRow("application/x-bzip2") << QString::fromLatin1("application/x-bzip2") << KCompressionDevice::BZip2;
#else
    QTest::newRow("application/x-bzip") << QString::fromLatin1("application/x-bzip") << KCompressionDevice::None;
    QTest::newRow("application/x-bzip2") << QString::fromLatin1("application/x-bzip2") << KCompressionDevice::None;
#endif
    // indirect compressed mimetypes
    QTest::newRow("application/x-gzdvi") << QString::fromLatin1("application/x-gzdvi") << KCompressionDevice::GZip;

    // non-compressed mimetypes
    QTest::newRow("text/plain") << QString::fromLatin1("text/plain") << KCompressionDevice::None;
    QTest::newRow("application/x-tar") << QString::fromLatin1("application/x-tar") << KCompressionDevice::None;
}

void KFilterTest::test_findFilterByMimeType()
{
    QFETCH(QString, mimeType);
    QFETCH(KCompressionDevice::CompressionType, type);

    KCompressionDevice::CompressionType compressionType = KFilterDev::compressionTypeForMimeType(mimeType);
    QCOMPARE(compressionType, type);
}

static void getCompressedData(QByteArray &data, QByteArray &compressedData)
{
    data = "Hello world, this is a test for deflate, from bug 114830 / 117683";
    compressedData.resize(long(data.size() * 1.1f) + 12L); // requirements of zlib::compress2
    unsigned long out_bufferlen = compressedData.size();
    const int ret = compress2((Bytef *)compressedData.data(), &out_bufferlen, (const Bytef *)data.constData(), data.size(), 1);
    QCOMPARE(ret, Z_OK);
    compressedData.resize(out_bufferlen);
}

void KFilterTest::test_deflateWithZlibHeader()
{
    QByteArray data, deflatedData;
    getCompressedData(data, deflatedData);

#if 0 // Can't use KFilterDev for this, we need to call KGzipFilter::init(QIODevice::ReadOnly, KGzipFilter::ZlibHeader);
    QBuffer buffer(&deflatedData);
    QIODevice *flt = KFilterDev::device(&buffer, "application/x-gzip", false);
    static_cast<KFilterDev *>(flt)->setSkipHeaders();
    bool ok = flt->open(QIODevice::ReadOnly);
    QVERIFY(ok);
    const QByteArray read = flt->readAll();
#else
    // Copied from HTTPFilter (which isn't linked into any kdelibs library)
    KFilterBase *mFilterDevice = KCompressionDevice::filterForCompressionType(KCompressionDevice::GZip);
    mFilterDevice->setFilterFlags(KFilterBase::ZlibHeaders);
    mFilterDevice->init(QIODevice::ReadOnly);

    mFilterDevice->setInBuffer(deflatedData.constData(), deflatedData.size());
    char buf[8192];
    mFilterDevice->setOutBuffer(buf, sizeof (buf));
    KFilterBase::Result result = mFilterDevice->uncompress();
    QCOMPARE(result, KFilterBase::End);
    const int bytesOut = sizeof (buf) - mFilterDevice->outBufferAvailable();
    QVERIFY(bytesOut);
    QByteArray read(buf, bytesOut);
    mFilterDevice->terminate();
    delete mFilterDevice;
#endif
    QCOMPARE(QString::fromLatin1(read.constData()), QString::fromLatin1(data.constData())); // more readable output than the line below
    QCOMPARE(read, data);

    // For the same test with HTTPFilter: see httpfiltertest.cpp
}

void KFilterTest::test_pushData() // ### UNFINISHED
{
    // HTTPFilter says KFilterDev doesn't support the case where compressed data
    // is arriving in chunks. Let's test that.
    QFile file(pathgz);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray compressed = file.readAll();
    const int firstChunkSize = compressed.size() / 2;
    QByteArray firstData(compressed.constData(), firstChunkSize);
    QBuffer inBuffer(&firstData);
    QVERIFY(inBuffer.open(QIODevice::ReadWrite));
    KCompressionDevice::CompressionType type = KFilterDev::compressionTypeForMimeType(QString::fromLatin1("application/x-gzip"));
    KCompressionDevice flt(&inBuffer, false, type);
    QVERIFY(flt.open(QIODevice::ReadOnly));
    QByteArray read = flt.readAll();
    qDebug() << QString::fromLatin1(read.constData());

    // And later...
    inBuffer.write(QByteArray(compressed.data() + firstChunkSize, compressed.size() - firstChunkSize));
    QCOMPARE(inBuffer.data().size(), compressed.size());
    read += flt.readAll();
    qDebug() << QString::fromLatin1(read.constData());
    // ### indeed, doesn't work currently. So we use HTTPFilter instead, for now.
}

void KFilterTest::test_saveFile_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<KCompressionDevice::CompressionType>("compressionType");

    QTest::newRow("gz") << "test_saveFile.gz" << KCompressionDevice::GZip;
    QTest::newRow("none") << "test_saveFile" << KCompressionDevice::None;
}

void KFilterTest::test_saveFile()
{
    QFETCH(QString, fileName);
    QFETCH(KCompressionDevice::CompressionType, compressionType);

    int numLines = 1000;
    const QString lineTemplate = QStringLiteral("Hello world, this is the text for line %1");
    const QString currentdir = QDir::currentPath();
    const QString outFile = QDir::currentPath() + '/' + fileName;
    {
        QSaveFile file(outFile);
        file.setDirectWriteFallback(true);
        QVERIFY(file.open(QIODevice::WriteOnly));
        KCompressionDevice device(&file, false, compressionType);
        QVERIFY(device.open(QIODevice::WriteOnly));
        QTextStream stream(&device);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));
        for (int i = 0 ; i < numLines ; ++i) {
            stream << lineTemplate.arg(i);
            stream << QString("\n");
        }
        stream.flush();
        QCOMPARE(stream.status(), QTextStream::Ok);
        //device.write("The data to be compressed");
        device.close();
        QVERIFY(file.commit());
    }
    QVERIFY(QFile::exists(outFile));
    KCompressionDevice reader(outFile, compressionType);
    QVERIFY(reader.open(QIODevice::ReadOnly));
    QString expectedFullData;
    for (int i = 0 ; i < numLines ; ++i) {
        QCOMPARE(QString::fromUtf8(reader.readLine()), QString(lineTemplate.arg(i) + '\n'));
        expectedFullData += QString(lineTemplate.arg(i) + '\n');
    }
    KFilterDev otherReader(outFile);
    QVERIFY(otherReader.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromLatin1(otherReader.readAll()), expectedFullData);
    QVERIFY(otherReader.atEnd());
}

void KFilterTest::test_twofilesgztogether()
{
    // Reported as 232843
    // twofiles generated with
    // echo foo > foo; echo bar > bar ; gzip -c foo > twofiles.gz; gzip -c bar >> twofiles.gz
    // as documented in the gzip manpage
    QString data = QFINDTESTDATA("data/twofiles.gz");
    KFilterDev dev(data);
    QVERIFY(dev.open(QIODevice::ReadOnly));
    QByteArray extractedData = dev.readAll();
    QByteArray expectedData{"foo\nbar\n"};
    QCOMPARE(extractedData, expectedData);
}

void KFilterTest::test_threefilesgztogether()
{
    // Generated similarly to the one above
    // This catches the case where there's more than two streams available in the same buffer fed to KGzipFilter
    QString data = QFINDTESTDATA("data/threefiles.gz");
    KFilterDev dev(data);
    QVERIFY(dev.open(QIODevice::ReadOnly));
    QByteArray extractedData = dev.readAll();
    QByteArray expectedData{"foo\nbar\nbaz\n"};
    QCOMPARE(extractedData, expectedData);
}
