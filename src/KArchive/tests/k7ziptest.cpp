/*
 *  SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "k7zip.h"
#include <stdio.h>
#include <QDebug>

void recursive_print(const KArchiveDirectory *dir, const QString &path)
{
    QStringList l = dir->entries();
    l.sort();
    QStringList::ConstIterator it = l.constBegin();
    for (; it != l.constEnd(); ++it) {
        const KArchiveEntry *entry = dir->entry((*it));
        printf("mode=%07o %s %s %s %s%s %lld isdir=%d\n", entry->permissions(), entry->date().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")).toLatin1().constData(),
               entry->user().toLatin1().constData(), entry->group().toLatin1().constData(), path.toLatin1().constData(), (*it).toLatin1().constData(),
               entry->isFile() ? static_cast<const KArchiveFile *>(entry)->size() : 0,
               entry->isDirectory());
        if (!entry->symLinkTarget().isEmpty()) {
            printf("  (symlink to %s)\n", qPrintable(entry->symLinkTarget()));
        }
        if (entry->isDirectory()) {
            recursive_print((KArchiveDirectory *)entry, path + (*it) + '/');
        }
        if (entry->isFile()) {
            const KArchiveFile *f = static_cast<const KArchiveFile *>(entry);
            QByteArray arr(f->data());
            qDebug() << "data" << arr;

            QIODevice *dev = f->createDevice();
            QByteArray contents = dev->readAll();
            qDebug() << "contents" << contents;
            delete dev;
        }
    }
}

// See karchivetest.cpp for the unittest that covers K7Zip.

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("\n"
               " Usage :\n"
               " ./k7ziptest /path/to/existing_file.7z       tests listing an existing .7z\n");
        return 1;
    }

    K7Zip k7z(argv[1]);

    if (!k7z.open(QIODevice::ReadOnly)) {
        printf("Could not open %s for reading\n", argv[1]);
        return 1;
    }

    const KArchiveDirectory *dir = k7z.directory();

    //printf("calling recursive_print\n");
    recursive_print(dir, QLatin1String(""));
    //printf("recursive_print called\n");

    k7z.close();

    return 0;
}

