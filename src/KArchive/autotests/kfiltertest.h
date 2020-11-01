/*
 *  SPDX-FileCopyrightText: 2002-2005 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KFILTERTEST_H
#define KFILTERTEST_H

#include <QObject>

class KFilterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void test_block_write();
    void test_block_read();
    void test_biggerWrites();
    void test_getch();
    void test_textstream();
    void test_readall();
    void test_uncompressed();
    void test_findFilterByMimeType_data();
    void test_findFilterByMimeType();
    void test_deflateWithZlibHeader();
    void test_pushData();
    void test_saveFile_data();
    void test_saveFile();
    void test_twofilesgztogether();
    void test_threefilesgztogether();

private:
    void test_block_write(const QString &fileName, const QByteArray &data);
    void test_block_read(const QString &fileName);
    void test_getch(const QString &fileName);
    void test_textstream(const QString &fileName);
    void test_readall(const QString &fileName, const QString &mimeType, const QByteArray &expectedData);

private:
    QString pathgz;
    QString pathbz2;
    QString pathxz;
    QString pathnone;
    QByteArray testData;
};

#endif
