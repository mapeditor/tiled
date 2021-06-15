/*
 * scriptfile.cpp
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

/*
 * This API and its implementation were based on the TextFile and BinaryFile
 * classes found in Qbs src/lib/corelib/jsextensions, Copyright (C) 2016 The Qt
 * Company Ltd, used under the terms of the GNU General Public License 2.0.
 *
 * Mostly adjusted for porting from QtScript to QJSEngine.
 */

#include "scriptfile.h"

#include "savefile.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJSEngine>
#include <QSaveFile>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QTextCodec>
#endif
#include <QTextStream>

#include <memory>

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


ScriptBinaryFile::ScriptBinaryFile()
{
    ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                     "BinaryFile constructor needs path of file to be opened."));
}

ScriptBinaryFile::ScriptBinaryFile(const QString &filePath, OpenMode mode)
{
    QIODevice::OpenMode m = QIODevice::NotOpen;
    if (mode & ReadOnly)
        m |= QIODevice::ReadOnly;
    if (mode & WriteOnly)
        m |= QIODevice::WriteOnly;

    if (m == QIODevice::WriteOnly && SaveFile::safeSavingEnabled())
        m_file.reset(new QSaveFile(filePath));
    else
        m_file.reset(new QFile(filePath));

    if (Q_UNLIKELY(!m_file->open(m))) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Unable to open file '%1': %2").arg(filePath,
                                                                                                             m_file->errorString()));
        m_file.reset();
    }
}

ScriptBinaryFile::~ScriptBinaryFile() = default;

QString ScriptBinaryFile::filePath() const
{
    if (checkForClosed())
        return {};
    return QFileInfo(m_file->fileName()).absoluteFilePath();
}

bool ScriptBinaryFile::atEof() const
{
    if (checkForClosed())
        return true;
    return m_file->atEnd();
}

qint64 ScriptBinaryFile::size() const
{
    if (checkForClosed())
        return -1;
    return m_file->size();
}

qint64 ScriptBinaryFile::pos() const
{
    if (checkForClosed())
        return -1;
    return m_file->pos();
}

void ScriptBinaryFile::resize(qint64 size)
{
    if (checkForClosed())
        return;
    if (Q_UNLIKELY(!m_file->resize(size))) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Could not resize '%1': %2").arg(m_file->fileName(),
                                                                                                          m_file->errorString()));
    }
}

void ScriptBinaryFile::seek(qint64 pos)
{
    if (checkForClosed())
        return;
    if (Q_UNLIKELY(!m_file->seek(pos))) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Could not seek '%1': %2").arg(m_file->fileName(),
                                                                                                        m_file->errorString()));
    }
}

QByteArray ScriptBinaryFile::read(qint64 size)
{
    if (checkForClosed())
        return {};
    const QByteArray data = m_file->read(size);
    if (Q_UNLIKELY(data.size() == 0 && m_file->error() != QFile::NoError)) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Could not read from '%1': %2").arg(m_file->fileName(),
                                                                                                             m_file->errorString()));
    }

    return data;
}

QByteArray ScriptBinaryFile::readAll()
{
    if (checkForClosed())
        return {};

    const QByteArray data = m_file->readAll();
    if (Q_UNLIKELY(data.size() == 0 && m_file->error() != QFile::NoError)) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Could not read from '%1': %2").arg(m_file->fileName(),
                                                                                                             m_file->errorString()));
    }

    return data;
}

void ScriptBinaryFile::write(const QByteArray &data)
{
    if (checkForClosed())
        return;

    const qint64 size = m_file->write(data);
    if (Q_UNLIKELY(size == -1)) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Could not write to '%1': %2").arg(m_file->fileName(),
                                                                                                            m_file->errorString()));
    }
}

void ScriptBinaryFile::commit()
{
    if (checkForClosed())
        return;

    bool ok = true;

    if (auto saveFile = qobject_cast<QSaveFile*>(m_file.get()))
        ok = saveFile->commit();
    else
        ok = m_file->flush();

    if (!ok) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Could not write to '%1': %2").arg(m_file->fileName(),
                                                                                                            m_file->errorString()));
    }

    close();
}

void ScriptBinaryFile::close()
{
    if (checkForClosed())
        return;

    m_file.reset();
}

bool ScriptBinaryFile::checkForClosed() const
{
    if (m_file)
        return false;

    ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                     "Access to BinaryFile object that was already closed."));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

ScriptTextFile::ScriptTextFile()
{
    ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                     "TextFile constructor needs path of file to be opened."));
}

ScriptTextFile::ScriptTextFile(const QString &filePath, OpenMode mode)
{
    QIODevice::OpenMode m = QIODevice::Text;
    if (mode & ReadOnly)
        m |= QIODevice::ReadOnly;
    if (mode & WriteOnly)
        m |= QIODevice::WriteOnly;
    if (mode & Append)
        m |= QIODevice::Append;

    if (m == (QIODevice::Text | QIODevice::WriteOnly) && SaveFile::safeSavingEnabled())
        m_file.reset(new QSaveFile(filePath));
    else
        m_file.reset(new QFile(filePath));

    if (Q_UNLIKELY(!m_file->open(m))) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Unable to open file '%1': %2").arg(filePath,
                                                                                                             m_file->errorString()));
        m_file.reset();
    } else {
        m_stream.reset(new QTextStream(m_file.get()));
    }
}

ScriptTextFile::~ScriptTextFile() = default;

QString ScriptTextFile::filePath() const
{
    if (checkForClosed())
        return {};
    return QFileInfo(m_file->fileName()).absoluteFilePath();
}

QString ScriptTextFile::codec() const
{
    if (checkForClosed())
        return {};
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    return QString::fromLatin1(m_stream->codec()->name());
#else
    return QString::fromLatin1(QStringConverter::nameForEncoding(m_stream->encoding()));
#endif
}

void ScriptTextFile::setCodec(const QString &codec)
{
    if (checkForClosed())
        return;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    m_stream->setCodec(codec.toLatin1());
#else
    auto encoding = QStringConverter::encodingForName(codec.toLatin1());
    if (!encoding.has_value()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Unsupported encoding: %1").arg(codec));
        return;
    }

    m_stream->setEncoding(encoding.value());
#endif
}

QString ScriptTextFile::readLine()
{
    if (checkForClosed())
        return {};
    return m_stream->readLine();
}

QString ScriptTextFile::readAll()
{
    if (checkForClosed())
        return {};
    return m_stream->readAll();
}

bool ScriptTextFile::atEof() const
{
    if (checkForClosed())
        return true;
    return m_stream->atEnd();
}

void ScriptTextFile::truncate()
{
    if (checkForClosed())
        return;
    m_file->resize(0);
    m_stream->reset();
}

void ScriptTextFile::write(const QString &string)
{
    if (checkForClosed())
        return;
    (*m_stream) << string;
}

void ScriptTextFile::writeLine(const QString &string)
{
    if (checkForClosed())
        return;
    (*m_stream) << string;
    (*m_stream) << '\n';
}

void ScriptTextFile::commit()
{
    if (checkForClosed())
        return;

    m_stream->flush();

    bool ok = true;

    if (auto saveFile = qobject_cast<QSaveFile*>(m_file.get()))
        ok = saveFile->commit();
    else
        ok = m_file->flush();

    if (!ok) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Could not write to '%1': %2").arg(m_file->fileName(),
                                                                                                            m_file->errorString()));
    }

    close();
}

void ScriptTextFile::close()
{
    if (checkForClosed())
        return;
    m_stream.reset();
    m_file.reset();
}

bool ScriptTextFile::checkForClosed() const
{
    if (m_file)
        return false;

    ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                     "Access to TextFile object that was already closed."));
    return true;
}

void registerFile(QJSEngine *jsEngine)
{
#if QT_VERSION >= 0x050800
    QJSValue globalObject = jsEngine->globalObject();
    globalObject.setProperty(QStringLiteral("TextFile"), jsEngine->newQMetaObject<ScriptTextFile>());
    globalObject.setProperty(QStringLiteral("BinaryFile"), jsEngine->newQMetaObject<ScriptBinaryFile>());
#endif
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptBinaryFile*)
Q_DECLARE_METATYPE(Tiled::ScriptTextFile*)

#include "scriptfile.moc"
