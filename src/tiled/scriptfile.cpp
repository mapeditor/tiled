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
 * This API and its implementation were based on the File, TextFile and BinaryFile
 * classes found in Qbs src/lib/corelib/jsextensions, Copyright (C) 2016 The Qt
 * Company Ltd, used under the terms of the GNU General Public License 2.0.
 *
 * Mostly adjusted for porting from QtScript to QJSEngine.
 */

#include "scriptfile.h"

#include "savefile.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJSEngine>
#include <QSaveFile>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QTextCodec>
#endif
#include <QTextStream>

#if defined(Q_OS_UNIX)
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif

#include <memory>

namespace Tiled {

class ScriptFile : public QObject
{
    Q_OBJECT

    // Hack to make the constants available on the instance
    Q_PROPERTY(int Dirs MEMBER Dirs CONSTANT)
    Q_PROPERTY(int Files MEMBER Files CONSTANT)
    Q_PROPERTY(int Drives MEMBER Drives CONSTANT)
    Q_PROPERTY(int NoSymLinks MEMBER NoSymLinks CONSTANT)
    Q_PROPERTY(int AllEntries MEMBER AllEntries CONSTANT)
    Q_PROPERTY(int TypeMask MEMBER TypeMask CONSTANT)
    Q_PROPERTY(int Readable MEMBER Readable CONSTANT)
    Q_PROPERTY(int Writable MEMBER Writable CONSTANT)
    Q_PROPERTY(int Executable MEMBER Executable CONSTANT)
    Q_PROPERTY(int PermissionMask MEMBER PermissionMask CONSTANT)
    Q_PROPERTY(int Modified MEMBER Modified CONSTANT)
    Q_PROPERTY(int Hidden MEMBER Hidden CONSTANT)
    Q_PROPERTY(int System MEMBER System CONSTANT)
    Q_PROPERTY(int AccessMask MEMBER AccessMask CONSTANT)
    Q_PROPERTY(int AllDirs MEMBER AllDirs CONSTANT)
    Q_PROPERTY(int CaseSensitive MEMBER CaseSensitive CONSTANT)
    Q_PROPERTY(int NoDot MEMBER NoDot CONSTANT)
    Q_PROPERTY(int NoDotDot MEMBER NoDotDot CONSTANT)
    Q_PROPERTY(int NoDotAndDotDot MEMBER NoDotAndDotDot CONSTANT)
    Q_PROPERTY(int NoFilter MEMBER NoFilter CONSTANT)
    Q_PROPERTY(int Name MEMBER Name CONSTANT)
    Q_PROPERTY(int Time MEMBER Time CONSTANT)
    Q_PROPERTY(int Size MEMBER Size CONSTANT)
    Q_PROPERTY(int Unsorted MEMBER Unsorted CONSTANT)
    Q_PROPERTY(int SortByMask MEMBER SortByMask CONSTANT)
    Q_PROPERTY(int DirsFirst MEMBER DirsFirst CONSTANT)
    Q_PROPERTY(int Reversed MEMBER Reversed CONSTANT)
    Q_PROPERTY(int IgnoreCase MEMBER IgnoreCase CONSTANT)
    Q_PROPERTY(int DirsLast MEMBER DirsLast CONSTANT)
    Q_PROPERTY(int LocaleAware MEMBER LocaleAware CONSTANT)
    Q_PROPERTY(int Type MEMBER Type CONSTANT)
    Q_PROPERTY(int NoSort MEMBER NoSort CONSTANT)

public:
    // See QDir
    enum Filter { Dirs        = 0x001,
                  Files       = 0x002,
                  Drives      = 0x004,
                  NoSymLinks  = 0x008,
                  AllEntries  = Dirs | Files | Drives,
                  TypeMask    = 0x00f,

                  Readable    = 0x010,
                  Writable    = 0x020,
                  Executable  = 0x040,
                  PermissionMask    = 0x070,

                  Modified    = 0x080,
                  Hidden      = 0x100,
                  System      = 0x200,

                  AccessMask  = 0x3F0,

                  AllDirs       = 0x400,
                  CaseSensitive = 0x800,
                  NoDot         = 0x2000,
                  NoDotDot      = 0x4000,
                  NoDotAndDotDot = NoDot | NoDotDot,

                  NoFilter = -1
    };

    enum SortFlag { Name        = 0x00,
                    Time        = 0x01,
                    Size        = 0x02,
                    Unsorted    = 0x03,
                    SortByMask  = 0x03,

                    DirsFirst   = 0x04,
                    Reversed    = 0x08,
                    IgnoreCase  = 0x10,
                    DirsLast    = 0x20,
                    LocaleAware = 0x40,
                    Type        = 0x80,
                    NoSort = -1
    };

    Q_INVOKABLE bool copy(const QString &sourceFilePath, const QString &targetFilePath) const;
    Q_INVOKABLE bool exists(const QString &filePath) const;
    Q_INVOKABLE QStringList directoryEntries(const QString &path, int filters = -1, int sortFlags = -1) const;
    Q_INVOKABLE QDateTime lastModified(const QString &filePath) const;
    Q_INVOKABLE bool makePath(const QString &path) const;
    Q_INVOKABLE bool move(const QString &sourceFile, const QString &targetFile, bool overwrite = true) const;
    Q_INVOKABLE bool remove(const QString &filePath) const;
};


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

///////////////////////////////////////////////////////////////////////////////

// adapted from qbs/src/lib/corelib/tools/fileinfo.cpp
bool removeFileRecursively(const QFileInfo &f, QString *errorMessage)
{
    if (!(f.isSymLink() || f.exists()))
        return true;

    if (f.isDir() && !f.isSymLink()) {
        const QDir dir(f.absoluteFilePath());

        // QDir::System is needed for broken symlinks.
        const auto fileInfos = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot
                                                 | QDir::Hidden | QDir::System);
        for (const QFileInfo &fi : fileInfos)
            removeFileRecursively(fi, errorMessage);
        QDir parent = f.absoluteDir();
        if (!parent.rmdir(f.fileName())) {
            if (!errorMessage->isEmpty())
                errorMessage->append(QLatin1Char('\n'));
            errorMessage->append(QCoreApplication::translate("Script Errors", "The directory '%1' could not be deleted.")
                                 .arg(QDir::toNativeSeparators(f.absoluteFilePath())));
            return false;
        }
    } else {
        QFile file(f.absoluteFilePath());
        file.setPermissions(f.permissions() | QFile::WriteUser);
        if (!file.remove()) {
            if (!errorMessage->isEmpty())
                errorMessage->append(QLatin1Char('\n'));
            errorMessage->append(QCoreApplication::translate("Script Errors", "The file '%1' could not be deleted.")
                                 .arg(QDir::toNativeSeparators(f.absoluteFilePath())));
            return false;
        }
    }
    return true;
}

#ifdef Q_OS_UNIX
/*!
 * Returns the stored link target of the symbolic link \a{filePath}.
 * Unlike QFileInfo::symLinkTarget, this will not make the link target an absolute path.
 */
static QByteArray storedLinkTarget(const QString &filePath)
{
    QByteArray result;

    const QByteArray nativeFilePath = QFile::encodeName(filePath);
    ssize_t len;
    while (true) {
        struct stat sb{};
        if (lstat(nativeFilePath.constData(), &sb)) {
            qWarning("storedLinkTarget: lstat for %s failed with error code %d",
                     nativeFilePath.constData(), errno);
            return {};
        }

        result.resize(sb.st_size);
        len = readlink(nativeFilePath.constData(), result.data(), sb.st_size + 1);
        if (len < 0) {
            qWarning("storedLinkTarget: readlink for %s failed with error code %d",
                     nativeFilePath.constData(), errno);
            return {};
        }

        if (len < sb.st_size) {
            result.resize(len);
            break;
        }
        if (len == sb.st_size)
            break;
    }

    return result;
}

static bool createSymLink(const QByteArray &path1, const QString &path2)
{
    const QByteArray newPath = QFile::encodeName(path2);
    unlink(newPath.constData());
    return symlink(path1.constData(), newPath.constData()) == 0;
}
#endif // Q_OS_UNIX

/*!
  Copies the directory specified by \a srcFilePath recursively to \a tgtFilePath.
  \a tgtFilePath will contain the target directory, which will be created. Example usage:

  \code
    QString error;
    book ok = copyRecursively("/foo/bar", "/foo/baz", &error);
    if (!ok)
      qDebug() << error;
  \endcode

  This will copy the contents of /foo/bar into to the baz directory under /foo,
  which will be created in the process.

  \return Whether the operation succeeded.
  \note Function was adapted from qbs/src/lib/corelib/tools/fileinfo.cpp
*/

static bool copyRecursively(const QString &srcFilePath,
                            const QString &tgtFilePath,
                            QString *errorMessage)
{
    QFileInfo srcFileInfo(srcFilePath);
    QFileInfo tgtFileInfo(tgtFilePath);
    const QString targetDirPath = tgtFileInfo.absoluteDir().path();
    if (!QDir::root().mkpath(targetDirPath)) {
        *errorMessage = QCoreApplication::translate("Script Errors", "The directory '%1' could not be created.")
                .arg(QDir::toNativeSeparators(targetDirPath));
        return false;
    }
#ifdef Q_OS_UNIX
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
    if (srcFileInfo.isSymLink()) {
#else
    if (srcFileInfo.isSymbolicLink()) {
#endif
        // For now, disable symlink preserving copying on Windows.
        // MS did a good job to prevent people from using symlinks - even if they are supported.
        if (!createSymLink(storedLinkTarget(srcFilePath), tgtFilePath)) {
            *errorMessage = QCoreApplication::translate("Script Errors", "The symlink '%1' could not be created.")
                    .arg(tgtFilePath);
            return false;
        }
        return true;
    }
#endif
    if (srcFileInfo.isDir()) {
        QDir sourceDir(srcFilePath);
        const QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs
                | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for (const QString &fileName : fileNames) {
            const QString newSrcFilePath = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath = tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath, errorMessage))
                return false;
        }
    } else {
        if (tgtFileInfo.exists() && srcFileInfo.lastModified() <= tgtFileInfo.lastModified())
            return true;
        QFile file(srcFilePath);
        QFile targetFile(tgtFilePath);
        if (targetFile.exists()) {
            targetFile.setPermissions(targetFile.permissions() | QFile::WriteUser);
            if (!targetFile.remove()) {
                *errorMessage = QCoreApplication::translate("Script Errors", "Could not remove file '%1': %2")
                        .arg(QDir::toNativeSeparators(tgtFilePath), targetFile.errorString());
            }
        }
        if (!file.copy(tgtFilePath)) {
            *errorMessage = QCoreApplication::translate("Script Errors", "Could not copy file '%1' to '%2': %3")
                .arg(QDir::toNativeSeparators(srcFilePath), QDir::toNativeSeparators(tgtFilePath),
                     file.errorString());
            return false;
        }
    }
    return true;
}

bool ScriptFile::copy(const QString &sourceFilePath, const QString &targetFilePath) const
{
    QString errorMessage;
    if (Q_UNLIKELY(!copyRecursively(sourceFilePath, targetFilePath, &errorMessage))) {
        ScriptManager::instance().throwError(errorMessage);
        return false;
    }
    return true;
}

bool ScriptFile::exists(const QString &filePath) const
{
    return QFileInfo::exists(filePath);
}

QStringList ScriptFile::directoryEntries(const QString &path, int filters, int sortFlags) const
{
    const QDir dir(path);
    return dir.entryList(static_cast<QDir::Filters>(filters),
                         static_cast<QDir::SortFlags>(sortFlags));
}

QDateTime ScriptFile::lastModified(const QString &filePath) const
{
    return QFileInfo(filePath).lastModified();
}

bool ScriptFile::makePath(const QString &path) const
{
    return QDir::root().mkpath(path);
}

bool ScriptFile::move(const QString &sourceFile, const QString &targetFile, bool overwrite) const
{
    auto throwError = [&](const QString &message) -> bool {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Could not move '%1' to '%2': %3")
                                             .arg(sourceFile, targetFile, message));
        return false;
    };

    if (Q_UNLIKELY(QFileInfo(sourceFile).isDir()))
        return throwError(QCoreApplication::translate("Script Errors", "Source file path is a directory."));

    if (Q_UNLIKELY(QFileInfo(targetFile).isDir()))
        return throwError(QCoreApplication::translate("Script Errors", "Destination file path is a directory."));

    QFile f(targetFile);
    if (overwrite && f.exists() && !f.remove())
        return throwError(f.errorString());

    if (QFile::exists(targetFile))
        return throwError(QCoreApplication::translate("Script Errors", "Destination file exists."));

    QFile f2(sourceFile);
    if (Q_UNLIKELY(!f2.rename(targetFile)))
        return throwError(f2.errorString());

    return true;
}

bool ScriptFile::remove(const QString &filePath) const
{
    QString errorMessage;
    if (Q_UNLIKELY(!removeFileRecursively(QFileInfo(filePath), &errorMessage))) {
        ScriptManager::instance().throwError(errorMessage);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

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
    QJSValue globalObject = jsEngine->globalObject();
    globalObject.setProperty(QStringLiteral("File"), jsEngine->newQObject(new ScriptFile));

#if QT_VERSION >= 0x050800
    globalObject.setProperty(QStringLiteral("TextFile"), jsEngine->newQMetaObject<ScriptTextFile>());
    globalObject.setProperty(QStringLiteral("BinaryFile"), jsEngine->newQMetaObject<ScriptBinaryFile>());
#endif
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptBinaryFile*)
Q_DECLARE_METATYPE(Tiled::ScriptTextFile*)

#include "scriptfile.moc"
