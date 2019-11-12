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
#include <QSaveFile>
#include <QTextCodec>
#include <QTextStream>

namespace Tiled {

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
}

void ScriptBinaryFile::close()
{
    if (checkForClosed())
        return;

    m_file->reset();
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

    if (m == (QIODevice::WriteOnly & QIODevice::Text) && SaveFile::safeSavingEnabled())
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
    return QString::fromLatin1(m_stream->codec()->name());
}

void ScriptTextFile::setCodec(const QString &codec)
{
    if (checkForClosed())
        return;
    m_stream->setCodec(codec.toLatin1());
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

} // namespace Tiled
