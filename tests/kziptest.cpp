/*
 *  SPDX-FileCopyrightText: 2002-2013 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kzip.h"
#include "kcompressiondevice.h"
#include <stdio.h>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>

void recursive_print(const KArchiveDirectory *dir, const QString &path)
{
    const QStringList lst = dir->entries();
    for (const QString &it : lst) {
        const KArchiveEntry *entry = dir->entry(it);
        printf("mode=%07o %s %s \"%s%s\" size: %lld pos: %lld isdir=%d%s", entry->permissions(),
               entry->user().toLatin1().constData(), entry->group().toLatin1().constData(),
               path.toLatin1().constData(), it.toLatin1().constData(),
               entry->isDirectory() ? 0 : (static_cast<const KArchiveFile *>(entry))->size(),
               entry->isDirectory() ? 0 : (static_cast<const KArchiveFile *>(entry))->position(),
               entry->isDirectory(),
               entry->symLinkTarget().isEmpty() ? "" : QStringLiteral(" symlink: %1").arg(entry->symLinkTarget()).toLatin1().constData());

        //    if (!entry->isDirectory()) printf("%d", (static_cast<const KArchiveFile *>(entry))->size());
        printf("\n");
        if (entry->isDirectory()) {
            recursive_print(static_cast<const KArchiveDirectory *>(entry), path + it + '/');
        }
    }
}

void recursive_transfer(const KArchiveDirectory *dir,
                        const QString &path, KZip *zip)
{
    const QStringList lst = dir->entries();
    for (const QString &it : lst) {
        const KArchiveEntry *e = dir->entry(it);
        qDebug() << "actual file: " << e->name();
        if (e->isFile()) {
            Q_ASSERT(e && e->isFile());
            const KArchiveFile *f = static_cast<const KArchiveFile *>(e);
            printf("FILE=%s\n", qPrintable(e->name()));

            QByteArray arr(f->data());
            printf("SIZE=%i\n", arr.size());
            QString str(arr);
            printf("DATA=%s\n", qPrintable(str));

            if (e->symLinkTarget().isEmpty()) {
                zip->writeFile(path + e->name(), arr);
            } else {
                zip->writeSymLink(path + e->name(), e->symLinkTarget());
            }
        } else if (e->isDirectory()) {
            recursive_transfer(static_cast<const KArchiveDirectory *>(e),
                               path + e->name() + '/', zip);
        }
    }
}

static int doList(const QString &fileName)
{
    KZip zip(fileName);
    if (!zip.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << fileName << "for reading. ZIP file doesn't exist or is invalid.";
        return 1;
    }
    const KArchiveDirectory *dir = zip.directory();
    recursive_print(dir, QString());
    zip.close();
    return 0;
}

static int doPrintAll(const QString &fileName)
{
    KZip zip(fileName);
    qDebug() << "Opening zip file";
    if (!zip.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << fileName << "for reading. ZIP file doesn't exist or is invalid.";
        return 1;
    }
    const KArchiveDirectory *dir = zip.directory();
    qDebug() << "Listing toplevel of zip file";
    const QStringList lst = dir->entries();
    for (const QString &it : lst) {
        const KArchiveEntry *e = dir->entry(it);
        qDebug() << "Printing" << it;
        if (e->isFile()) {
            Q_ASSERT(e && e->isFile());
            const KArchiveFile *f = static_cast<const KArchiveFile *>(e);
            const QByteArray data(f->data());
            printf("SIZE=%i\n", data.size());
            QString str = QString::fromUtf8(data);
            printf("DATA=%s\n", qPrintable(str));
        }
    }
    zip.close();
    return 0;
}

static int doSave(const QString &fileName)
{
    KZip zip(fileName);
    if (!zip.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open" << fileName << "for writing";
        return 1;
    }

    const QByteArray data = "This is the data for the main file";
    bool writeOk = zip.writeFile(QStringLiteral("maindoc.txt"), data);
    if (!writeOk) {
        qWarning() << "Write error (main file)";
        return 1;
    }
    const QByteArray data2 = "This is the data for the other file";
    writeOk = zip.writeFile(QStringLiteral("subdir/other.txt"), data2);
    if (!writeOk) {
        qWarning() << "Write error (other file)";
        return 1;
    }
    //writeOk = zip.addLocalFile("David.jpg", "picture.jpg");
    //if (!writeOk) {
    //    qWarning() << "Write error (picture)";
    //    return 1;
    //}
    return 0;
}

static int doLoad(const QString &fileName)
{
    KZip zip(fileName);
    if (!zip.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << fileName << "for reading. ZIP file doesn't exist or is invalid.";
        return 1;
    }
    const KArchiveDirectory *dir = zip.directory();
    const KArchiveEntry *mainEntry = dir->entry(QStringLiteral("maindoc.txt"));
    Q_ASSERT(mainEntry && mainEntry->isFile());
    const KArchiveFile *mainFile = static_cast<const KArchiveFile *>(mainEntry);
    qDebug() << "maindoc.txt:" << mainFile->data();
    return 0;
}

static int doPrint(const QString &fileName, const QString &entryName)
{
    KZip zip(fileName);
    if (!zip.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << fileName << "for reading. ZIP file doesn't exist or is invalid.";
        return 1;
    }
    const KArchiveDirectory *dir = zip.directory();
    const KArchiveEntry *e = dir->entry(entryName);
    Q_ASSERT(e && e->isFile());
    const KArchiveFile *f = static_cast<const KArchiveFile *>(e);

    const QByteArray arr(f->data());
    printf("SIZE=%i\n", arr.size());
    QString str = QString::fromUtf8(arr);
    printf("%s", qPrintable(str));
    return zip.close() ? 0 : 1 /*error*/;
}

static int doUpdate(const QString &archiveName, const QString &fileName)
{
    KZip zip(archiveName);
    if (!zip.open(QIODevice::ReadWrite)) {
        qWarning() << "Could not open" << archiveName << "for read/write";
        return 1;
    }

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << fileName << "for reading.";
        return 1;
    }

    zip.writeFile(fileName, f.readAll());
    return zip.close() ? 0 : 1 /*error*/;
}

static int doTransfer(const QString &sourceFile, const QString &destFile)
{
    KZip zip1(sourceFile);
    KZip zip2(destFile);
    if (!zip1.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << sourceFile << "for reading. ZIP file doesn't exist or is invalid.";
        return 1;
    }
    if (!zip2.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open" << destFile << "for writing";
        return 1;
    }
    const KArchiveDirectory *dir1 = zip1.directory();

    recursive_transfer(dir1, QLatin1String(""), &zip2);

    zip1.close();
    zip2.close();
    return 0;
}

static bool save(QIODevice *device)
{
    const QByteArray data = "This is some text that will be compressed.\n";
    const int written = device->write(data);
    if (written != data.size()) {
        qWarning() << "Error writing data";
        return 1;
    }
    // success
    return 0;
}

static int doCompress(const QString &fileName)
{
    KCompressionDevice device(fileName, KCompressionDevice::BZip2);
    if (!device.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open" << fileName << "for writing";
        return 1;
    }

    return save(&device);
}

static bool load(QIODevice *device)
{
    const QByteArray data = device->readAll();
    printf("%s", data.constData());
    return true;
}

static int doUncompress(const QString &fileName)
{
    KCompressionDevice device(fileName, KCompressionDevice::BZip2);
    if (!device.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << fileName << "for reading";
        return 1;
    }
    return load(&device);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        // ###### Note: please consider adding new tests to karchivetest (so that they can be automated)
        // rather than here (interactive)
        printf("\n"
               " Usage :\n"
               " ./kziptest list /path/to/existing_file.zip       tests listing an existing zip\n"
               " ./kziptest print-all file.zip                    prints contents of all files.\n"
               " ./kziptest print file.zip filename               prints contents of one file.\n"
               " ./kziptest update file.zip filename              update filename in file.zip.\n"
               " ./kziptest save file.zip                         save file.\n"
               " ./kziptest load file.zip                         load file.\n"
               " ./kziptest write file.bz2                        write compressed file.\n"
               " ./kziptest read file.bz2                         read uncompressed file.\n"
              );
        return 1;
    }
    QCoreApplication app(argc, argv);
    QString command = argv[1];
    if (command == QLatin1String("list")) {
        return doList(QFile::decodeName(argv[2]));
    } else if (command == QLatin1String("print-all")) {
        return doPrintAll(QFile::decodeName(argv[2]));
    } else if (command == QLatin1String("print")) {
        if (argc != 4) {
            printf("usage: kziptest print archivename filename");
            return 1;
        }
        return doPrint(QFile::decodeName(argv[2]), argv[3]);
    } else if (command == QLatin1String("save")) {
        return doSave(QFile::decodeName(argv[2]));
    } else if (command == QLatin1String("load")) {
        return doLoad(QFile::decodeName(argv[2]));
    } else if (command == QLatin1String("write")) {
        return doCompress(QFile::decodeName(argv[2]));
    } else if (command == QLatin1String("read")) {
        return doUncompress(QFile::decodeName(argv[2]));
    } else if (command == QLatin1String("update")) {
        if (argc != 4) {
            printf("usage: kziptest update archivename filename");
            return 1;
        }
        return doUpdate(QFile::decodeName(argv[2]), QFile::decodeName(argv[3]));
    } else if (command == QLatin1String("transfer")) {
        if (argc != 4) {
            printf("usage: kziptest transfer sourcefile destfile");
            return 1;
        }
        return doTransfer(QFile::decodeName(argv[2]), QFile::decodeName(argv[3]));
    } else {
        printf("Unknown command\n");
    }
}
