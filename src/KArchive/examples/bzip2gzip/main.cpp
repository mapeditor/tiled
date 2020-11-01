/*  This file is part of the KDE project

    SPDX-FileCopyrightText: 2014 Maarten De Meyer <de.meyer.maarten@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/

/*
 * bzip2gzip
 * This example shows the usage of KCompressionDevice.
 * It converts BZip2 files to GZip archives.
 *
 * api: KCompressionDevice(QIODevice * inputDevice, bool autoDeleteInputDevice, CompressionType type)
 * api: KCompressionDevice(const QString & fileName, CompressionType type)
 * api: QIODevice::readAll()
 * api: QIODevice::read(qint64 maxSize)
 * api: QIODevice::write(const QByteArray &data)
 *
 * Usage: ./bzip2gzip <archive.bz2>
*/

#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QFileInfo>

#include <KCompressionDevice>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args(app.arguments());

    if (args.size() != 2) {
        qWarning("Usage: ./bzip2gzip <archive.bz2>");
        return 1;
    }

    QString inputFile = args.at(1);
    QFile file(inputFile);
    QFileInfo info(inputFile);

    if (info.suffix() != QStringLiteral("bz2")) {
        qCritical("Error: not a valid BZip2 file!");
        return 1;
    }

    //@@snippet_begin(kcompressiondevice_example)
    // Open the input archive
    KCompressionDevice input(&file, false, KCompressionDevice::BZip2);
    input.open(QIODevice::ReadOnly);

    QString outputFile = (info.completeBaseName() + QStringLiteral(".gz"));

    // Open the new output file
    KCompressionDevice output(outputFile, KCompressionDevice::GZip);
    output.open(QIODevice::WriteOnly);

    while (!input.atEnd()) {
        // Read and uncompress the data
        QByteArray data = input.read(512);

        // Write data like you would to any other QIODevice
        output.write(data);
    }

    input.close();
    output.close();
    //@@snippet_end

    return 0;
}
