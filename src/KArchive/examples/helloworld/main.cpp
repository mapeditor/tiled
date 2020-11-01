/*  This file is part of the KDE project

    SPDX-FileCopyrightText: 2013 Maarten De Meyer <de.meyer.maarten@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/

/*
 * HelloWorld
 *
 * Example to show very basic usage of KArchive with CMake
 *
 * Usage:
 *      mkdir build && cd build
 *      cmake ..
 *      make
 *      ./helloworld
*/

#include <QDebug>
#include <kzip.h>

int main()
{
    //@@snippet_begin(helloworld)
    // Create a zip archive
    KZip archive(QStringLiteral("hello.zip"));

    // Open our archive for writing
    if (archive.open(QIODevice::WriteOnly)) {

        // The archive is open, we can now write data
        archive.writeFile(QStringLiteral("world"),                       // File name
                          QByteArray("The whole world inside a hello."), // Data
                          0100644,                                       // Permissions
                          QStringLiteral("owner"),                       // Owner
                          QStringLiteral("users"));                      // Group

        // Don't forget to close!
        archive.close();
    }

    if (archive.open(QIODevice::ReadOnly)) {
        const KArchiveDirectory *dir = archive.directory();

        const KArchiveEntry *e = dir->entry("world");
        if (!e) {
            qDebug() << "File not found!";
            return -1;
        }
        const KArchiveFile *f = static_cast<const KArchiveFile *>(e);
        QByteArray arr(f->data());
        qDebug() << arr; // the file contents

        // To avoid reading everything into memory in one go, we can use createDevice() instead
        QIODevice *dev = f->createDevice();
        while (!dev->atEnd()) {
            qDebug() << dev->readLine();
        }
        delete dev;
    }
    //@@snippet_end

    return 0;
}
