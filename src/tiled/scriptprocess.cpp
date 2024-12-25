/*
 * scriptprocess.cpp
 * Copyright 2020, David Konsumer <konsumer@jetboystudio.com>
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
 * This API and its implementation were based on the Process class found in Qbs
 * src/lib/corelib/jsextensions, Copyright (C) 2016 The Qt Company Ltd, used
 * under the terms of the GNU General Public License 2.0.
 *
 * Mostly adjusted for porting from QtScript to QJSEngine.
 */

#include "scriptmanager.h"

#include <QString>
#include <QProcess>
#include <QProcessEnvironment>
#include <QJSEngine>
#include <QCoreApplication>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

#include <memory>

namespace Tiled {

class ScriptProcess : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString workingDirectory READ workingDirectory WRITE setWorkingDirectory);
    Q_PROPERTY(bool atEnd READ atEnd)
    Q_PROPERTY(int exitCode READ exitCode)
    Q_PROPERTY(QString codec READ codec WRITE setCodec)

public:
    Q_INVOKABLE ScriptProcess();
    ~ScriptProcess() override;

    Q_INVOKABLE QString getEnv(const QString &name);
    Q_INVOKABLE void setEnv(const QString &name, const QString &value);

    QString codec() const;
    void setCodec(const QString &codec);

    Q_INVOKABLE QString workingDirectory();
    Q_INVOKABLE void setWorkingDirectory(const QString &dir);

    Q_INVOKABLE bool start(const QString &program, const QStringList &arguments = {});
    Q_INVOKABLE int exec(const QString &program, const QStringList &arguments = {}, bool throwOnError = true);
    Q_INVOKABLE void close();
    Q_INVOKABLE bool waitForFinished(int msecs = 30000);
    Q_INVOKABLE void terminate();
    Q_INVOKABLE void kill();

    Q_INVOKABLE QString readLine();
    bool atEnd() const;
    Q_INVOKABLE QString readStdOut();
    Q_INVOKABLE QString readStdErr();
    Q_INVOKABLE void closeWriteChannel();
    Q_INVOKABLE void write(const QString &string);
    Q_INVOKABLE void writeLine(const QString &string);

    int exitCode() const;

private:
    bool checkForClosed() const;

    QByteArray encode(const QString &string) const;
    QString decode(const QByteArray &bytes) const;

    std::unique_ptr<QProcess> m_process;
    QProcessEnvironment m_environment;
    QString m_workingDirectory;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QTextCodec *m_codec;
#else
    QStringConverter::Encoding m_encoding = QStringConverter::System;
#endif
};

ScriptProcess::ScriptProcess()
    : m_process(new QProcess)
    , m_environment(QProcessEnvironment::systemEnvironment())
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    , m_codec(QTextCodec::codecForName("UTF-8"))
#endif
{
}

ScriptProcess::~ScriptProcess()
{
    close();
}

QString ScriptProcess::getEnv(const QString &name)
{
    return m_environment.value(name);
}

void ScriptProcess::setEnv(const QString &name, const QString &value)
{
    m_environment.insert(name, value);
}

QString ScriptProcess::codec() const
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    return QString::fromLatin1(m_codec->name());
#else
    return QString::fromLatin1(QStringConverter::nameForEncoding(m_encoding));
#endif
}

void ScriptProcess::setCodec(const QString &codec)
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    if (const auto newCodec = QTextCodec::codecForName(codec.toLatin1())) {
        m_codec = newCodec;
    } else {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Unsupported encoding: %1").arg(codec));
    }
#else
    auto encoding = QStringConverter::encodingForName(codec.toLatin1());
    if (!encoding.has_value()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                         "Unsupported encoding: %1").arg(codec));
        return;
    }
    m_encoding = encoding.value();
#endif
}

QString ScriptProcess::workingDirectory()
{
    return m_workingDirectory;
}

void ScriptProcess::setWorkingDirectory(const QString &dir)
{
    m_workingDirectory = dir;
}

bool ScriptProcess::start(const QString &program, const QStringList &arguments)
{
    if (checkForClosed())
        return false;

    if (!m_workingDirectory.isEmpty())
        m_process->setWorkingDirectory(m_workingDirectory);

    m_process->setProcessEnvironment(m_environment);
    m_process->start(program, arguments);
    return m_process->waitForStarted();
}

int ScriptProcess::exec(const QString &program, const QStringList &arguments, bool throwOnError)
{
    if (checkForClosed())
        return -1;

    if (!start(program, arguments)) {
        if (throwOnError) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Error running %1: %2")
                                                 .arg(program, m_process->errorString()));
        }
        return -1;
    }
    m_process->closeWriteChannel();
    m_process->waitForFinished(-1);

    if (throwOnError) {
        if (m_process->error() != QProcess::UnknownError && m_process->error() != QProcess::Crashed) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Error running %1: %2")
                                                 .arg(program, m_process->errorString()));
        } else if (m_process->exitStatus() == QProcess::CrashExit || m_process->exitCode() != 0) {
            QString errorMessage = m_process->error() == QProcess::Crashed
                    ? QCoreApplication::translate("Script Errors", "Error running '%1': %2").arg(program, m_process->errorString())
                    : QCoreApplication::translate("Script Errors", "Process '%1 %2' finished with exit code %3.")
                      .arg(program, arguments.join(QLatin1Char(' ')))
                      .arg(m_process->exitCode());
            const QString stdOut = readStdOut().trimmed();
            if (!stdOut.isEmpty()) {
                errorMessage.append(QLatin1Char('\n'))
                        .append(QCoreApplication::translate("Script Errors", "The standard output was:"))
                        .append(QLatin1Char('\n'))
                        .append(stdOut);
            }
            const QString stdErr = readStdErr().trimmed();
            if (!stdErr.isEmpty()) {
                errorMessage.append(QLatin1Char('\n'))
                        .append(QCoreApplication::translate("Script Errors", "The standard error output was:"))
                        .append(QLatin1Char('\n'))
                        .append(stdErr);
            }
            ScriptManager::instance().throwError(errorMessage);
        }
    }
    if (m_process->error() != QProcess::UnknownError)
        return -1;

    return m_process->exitCode();
}

void ScriptProcess::close()
{
    if (checkForClosed())
        return;

    m_process.reset();
}

bool ScriptProcess::waitForFinished(int msecs)
{
    if (checkForClosed())
        return false;

    return m_process->waitForFinished(msecs);
}

void ScriptProcess::terminate()
{
    if (checkForClosed())
        return;

    m_process->terminate();
}

void ScriptProcess::kill()
{
    if (checkForClosed())
        return;

    m_process->kill();
}

QString ScriptProcess::readLine()
{
    if (checkForClosed())
        return {};

    QString result = decode(m_process->readLine());
    if (!result.isEmpty() && result.at(result.size() - 1) == QLatin1Char('\n'))
        result.chop(1);
    return result;
}

bool ScriptProcess::atEnd() const
{
    if (checkForClosed())
        return true;

    return m_process->atEnd();
}

QString ScriptProcess::readStdOut()
{
    if (checkForClosed())
        return {};

    return decode(m_process->readAllStandardOutput());
}

QString ScriptProcess::readStdErr()
{
    if (checkForClosed())
        return {};

    return decode(m_process->readAllStandardError());
}

void ScriptProcess::closeWriteChannel()
{
    if (checkForClosed())
        return;

    m_process->closeWriteChannel();
}

void ScriptProcess::write(const QString &string)
{
    if (checkForClosed())
        return;

    m_process->write(encode(string));
}

void ScriptProcess::writeLine(const QString &string)
{
    if (checkForClosed())
        return;

    m_process->write(encode(string));
    m_process->putChar('\n');
}

int ScriptProcess::exitCode() const
{
    if (checkForClosed())
        return -1;

    return m_process->exitCode();
}

bool ScriptProcess::checkForClosed() const
{
    if (m_process)
        return false;

    ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors",
                                                                     "Access to Process object that was already closed."));
    return true;
}

QByteArray ScriptProcess::encode(const QString &string) const
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    return m_codec->fromUnicode(string);
#else
    return QStringEncoder(m_encoding).encode(string);
#endif
}

QString ScriptProcess::decode(const QByteArray &bytes) const
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    return m_codec->toUnicode(bytes);
#else
    return QStringDecoder(m_encoding).decode(bytes);
#endif
}


void registerProcess(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("Process"),
                                         jsEngine->newQMetaObject<ScriptProcess>());
}

} // namespace Tiled

#include "scriptprocess.moc"
