/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLIMITEDIODEVICETEST_H
#define KLIMITEDIODEVICETEST_H

#include <QByteArray>
#include <QBuffer>
#include <QList>
#include <QObject>

struct ChunkData
{
    QByteArray data;
    int offset;
};

class KLimitedIODeviceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testReadChunks_data();
    void testReadChunks();
    void testSeeking();

private:
    void addChunk(const QByteArray &chunk);

    QByteArray m_data;
    QBuffer m_buffer;
    QList<ChunkData> m_chunks;
};

#endif
