/*
 * scriptfile.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>

#include <memory>

class QFileDevice;
class QTextStream;

namespace Tiled {

class ScriptBinaryFile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString filePath READ filePath)
    Q_PROPERTY(bool atEof READ atEof)
    Q_PROPERTY(qint64 size READ size)
    Q_PROPERTY(qint64 pos READ pos)

public:
    enum OpenMode {
        ReadOnly = 1,
        WriteOnly = 2,
        ReadWrite = ReadOnly | WriteOnly
    };
    Q_ENUM(OpenMode)

    Q_INVOKABLE ScriptBinaryFile(); // workaround for Qt issue
    Q_INVOKABLE ScriptBinaryFile(const QString &filePath,
                                 OpenMode mode = ReadOnly);
    ~ScriptBinaryFile() override;

    QString filePath() const;
    bool atEof() const;
    qint64 size() const;
    qint64 pos() const;

    Q_INVOKABLE void resize(qint64 size);
    Q_INVOKABLE void seek(qint64 pos);
    Q_INVOKABLE QByteArray read(qint64 size);
    Q_INVOKABLE QByteArray readAll();
    Q_INVOKABLE void write(const QByteArray &data);
    Q_INVOKABLE void commit();
    Q_INVOKABLE void close();

private:
    bool checkForClosed() const;

    std::unique_ptr<QFileDevice> m_file;
};

class ScriptTextFile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString filePath READ filePath)
    Q_PROPERTY(bool atEof READ atEof)
    Q_PROPERTY(QString codec READ codec WRITE setCodec)

public:
    enum OpenMode {
        ReadOnly = 1,
        WriteOnly = 2,
        ReadWrite = ReadOnly | WriteOnly,
        Append = 4
    };
    Q_ENUM(OpenMode)

    Q_INVOKABLE ScriptTextFile();   // workaround for Qt issue
    Q_INVOKABLE ScriptTextFile(const QString &filePath,
                               OpenMode mode = ReadOnly);
    ~ScriptTextFile() override;

    QString filePath() const;
    QString codec() const;
    void setCodec(const QString &codec);

    Q_INVOKABLE QString readLine();
    Q_INVOKABLE QString readAll();
    bool atEof() const;
    Q_INVOKABLE void truncate();
    Q_INVOKABLE void write(const QString &string);
    Q_INVOKABLE void writeLine(const QString &string);
    Q_INVOKABLE void commit();
    Q_INVOKABLE void close();

private:
    bool checkForClosed() const;

    std::unique_ptr<QFileDevice> m_file;
    std::unique_ptr<QTextStream> m_stream;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptBinaryFile*)
Q_DECLARE_METATYPE(Tiled::ScriptTextFile*)
