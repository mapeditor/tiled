/*  This file is part of the KDE project

    SPDX-FileCopyrightText: 2013 Maarten De Meyer <de.meyer.maarten@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/

/*
 * TarLocalFiles
 * This example shows how to add local files and directories to a KArchive
 *
 * api: addLocalFile(fileName, destName)
 * api: addLocalDirectory(dirName, destName)
 *
 * Usage: ./tarlocalfiles <file-1> <file-n>
*/

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <ktar.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList files(app.arguments());

    // Create or open an archive
    KTar archive(QStringLiteral("myFiles.tar.gz"));

    // Prepare the archive for writing.
    if (!archive.open(QIODevice::WriteOnly)) {
        // Failed to open file.
        return 1;
    }

    if (files.size() <= 1) {
        // No files given.
        qWarning("Usage: ./tarlocalfiles <file>");
        return 1;
    }

    for (int i = 1; i < files.size(); ++i) {
        QFileInfo localFileOrDir(files.at(i));

        if (localFileOrDir.isFile()) {
            QString name = localFileOrDir.fileName();
            archive.addLocalFile(name, name);
        } else if (localFileOrDir.isDir()) {
            QString name = QDir(files.at(i)).dirName();
            // Add this folder and all its contents
            archive.addLocalDirectory(name, name);
        }
    }

    archive.close();
    return 0;
}
