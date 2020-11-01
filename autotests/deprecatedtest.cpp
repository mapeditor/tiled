/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006, 2010 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2012 Mario Bensi <mbensi@ipsquad.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kzip.h>

#include <QTest>

static const char s_zipFileName[] = "deprecatedtest.zip";

class DeprecatedTest : public QObject
{
    Q_OBJECT

#if KARCHIVE_ENABLE_DEPRECATED_SINCE(5, 0)
private Q_SLOTS:
    void testKArchiveWriteFile()
    {
        KZip zip(s_zipFileName);

        QVERIFY(zip.open(QIODevice::WriteOnly));

        const QByteArray fileData("There could be a fire, if there is smoke.");
        const QString fileName = QStringLiteral("wisdom");
        QVERIFY(zip.writeFile(fileName, "konqi", "dragons", fileData.constData(), fileData.size()));

        QVERIFY(zip.close());

        QVERIFY(zip.open(QIODevice::ReadOnly));

        const KArchiveDirectory *dir = zip.directory();
        QVERIFY(dir != nullptr);
        const QStringList listing = dir->entries();
        QCOMPARE(listing.count(), 1);
        QCOMPARE(listing.at(0), fileName);
        const KArchiveEntry *entry = dir->entry(listing.at(0));
        QCOMPARE(entry->permissions(), mode_t(0100644));
        QVERIFY(!entry->isDirectory());
        const KArchiveFile *fileEntry = static_cast<const KArchiveFile *>(entry);
        QCOMPARE(fileEntry->size(), fileData.size());
        QCOMPARE(fileEntry->data(), fileData);
    }

    /**
     * @see QTest::cleanupTestCase()
     */
    void cleanupTestCase()
    {
        QFile::remove(s_zipFileName);
    }
#endif
};

QTEST_MAIN(DeprecatedTest)

#include <deprecatedtest.moc>
