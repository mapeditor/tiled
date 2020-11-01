/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2012 Mario Bensi <mbensi@ipsquad.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KARCHIVETEST_H
#define KARCHIVETEST_H

#include <QObject>
#include <config-compression.h>

class KArchiveTest : public QObject
{
    Q_OBJECT

    void setupData();
    void setup7ZipData();

private Q_SLOTS:
    void initTestCase();

    void testEmptyFilename();
    void testNullDevice();
    void testNonExistentFile();
    void testCreateTar_data();
    void testCreateTar();
    void testCreateTarXXX_data()
    {
        setupData();
    }
    void testCreateTarXXX();
    void testReadTar_data()
    {
        setupData();
    }
    void testReadTar();
    void testUncompress_data()
    {
        setupData();
    }
    void testUncompress();
    void testTarFileData_data()
    {
        setupData();
    }
    void testTarFileData();
    void testTarCopyTo_data()
    {
        setupData();
    }
    void testTarCopyTo();
    void testTarReadWrite_data()
    {
        setupData();
    }
    void testTarReadWrite();
    void testTarMaxLength_data();
    void testTarMaxLength();
    void testTarGlobalHeader();
    void testTarPrefix();
    void testTarDirectoryForgotten();
    void testTarEmptyFileMissingDir();
    void testTarRootDir();
    void testTarDirectoryTwice();
    void testTarIgnoreRelativePathOutsideArchive();
    void testTarLongNonASCIINames();
    void testTarShortNonASCIINames();

    void testCreateZip();
    void testCreateZipError();
    void testReadZipError();
    void testReadZip();
    void testZipFileData();
    void testZipCopyTo();
    void testZipMaxLength();
    void testZipWithNonLatinFileNames();
    void testZipWithOverwrittenFileName();
    void testZipAddLocalDirectory();
    void testZipReadRedundantDataDescriptor_data();
    void testZipReadRedundantDataDescriptor();
    void testZipDirectoryPermissions();
    void testZipUnusualButValid();
    void testZipDuplicateNames();
    void testZipWithinZip();

    void testRcc();

    void testAr();

#if HAVE_XZ_SUPPORT
    void testCreate7Zip_data()
    {
        setup7ZipData();
    }
    void testCreate7Zip();
    void testRead7Zip_data()
    {
        setup7ZipData();
    }
    void testRead7Zip();
    void test7ZipFileData_data()
    {
        setup7ZipData();
    }
    void test7ZipFileData();
    void test7ZipCopyTo_data()
    {
        setup7ZipData();
    }
    void test7ZipCopyTo();
    void test7ZipReadWrite_data()
    {
        setup7ZipData();
    }
    void test7ZipReadWrite();
    void test7ZipMaxLength_data()
    {
        setup7ZipData();
    }
    void test7ZipMaxLength();
#endif

    void cleanupTestCase();
};

#endif
