/*
 * scriptserver.cpp
 * Copyright 2026, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptserver.h"

#include "logginginterface.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

using namespace Tiled;

bool ScriptServer::forceEnabled = false;

static QString outputTypeToString(LoggingInterface::OutputType type)
{
    switch (type) {
    case LoggingInterface::INFO:
        return QStringLiteral("info");
    case LoggingInterface::WARNING:
        return QStringLiteral("warning");
    case LoggingInterface::ERROR:
        return QStringLiteral("error");
    }
    return QString();
}

static LoggingInterface::OutputType outputTypeFromString(const QString &type)
{
    if (type == QLatin1String("warning"))
        return LoggingInterface::WARNING;
    if (type == QLatin1String("error"))
        return LoggingInterface::ERROR;
    return LoggingInterface::INFO;
}

static QJsonObject toJson(const ScriptManager::EvaluationResult &result)
{
    QJsonObject object;

    if (result.hasResult) {
        object.insert(QStringLiteral("result"), result.result);
        object.insert(QStringLiteral("tempName"), result.tempName);
    }
    if (result.hasError())
        object.insert(QStringLiteral("error"), result.error);

    QJsonArray output;
    for (const auto &line : result.output) {
        output.append(QJsonObject {
            { QStringLiteral("type"), outputTypeToString(line.type) },
            { QStringLiteral("text"), line.text },
        });
    }
    object.insert(QStringLiteral("output"), output);

    return object;
}

static ScriptManager::EvaluationResult fromJson(const QJsonObject &object)
{
    ScriptManager::EvaluationResult result;

    if (object.contains(QLatin1String("result"))) {
        result.hasResult = true;
        result.result = object.value(QLatin1String("result")).toString();
        result.tempName = object.value(QLatin1String("tempName")).toString();
    }
    if (object.contains(QLatin1String("error")))
        result.error = object.value(QLatin1String("error")).toString();

    const QJsonArray output = object.value(QLatin1String("output")).toArray();
    for (const QJsonValue &value : output) {
        const QJsonObject line = value.toObject();
        result.output.append({
            outputTypeFromString(line.value(QLatin1String("type")).toString()),
            line.value(QLatin1String("text")).toString()
        });
    }

    return result;
}


ScriptServer::ScriptServer(QObject *parent)
    : QObject(parent)
    , mServer(new QLocalServer(this))
{
    // Only allow connections from the same user
    mServer->setSocketOptions(QLocalServer::UserAccessOption);

    connect(mServer, &QLocalServer::newConnection,
            this, &ScriptServer::newConnection);
}

ScriptServer::~ScriptServer()
{
    close();
}

QString ScriptServer::serverName()
{
    QString name = QStringLiteral("tiled-script-server");
#if defined(Q_OS_UNIX)
    name += QLatin1Char('-') + QString::number(::getuid());
#elif defined(Q_OS_WIN)
    name += QLatin1Char('-') + qEnvironmentVariable("USERNAME");
#endif
    return name;
}

bool ScriptServer::listen()
{
    if (mServer->isListening())
        return true;

    const QString name = serverName();

    // Don't take over from another instance that is already serving
    QLocalSocket probe;
    probe.connectToServer(name);
    if (probe.waitForConnected(100)) {
        probe.disconnectFromServer();
        INFO(tr("Script server already running in another Tiled instance"));
        return false;
    }

    // Remove any stale socket left behind by a crashed instance
    QLocalServer::removeServer(name);

    if (!mServer->listen(name)) {
        ERROR(tr("Failed to start script server: %1").arg(mServer->errorString()));
        return false;
    }

    INFO(tr("Script server listening at '%1'").arg(mServer->fullServerName()));
    return true;
}

void ScriptServer::close()
{
    if (!mServer->isListening())
        return;

    mServer->close();
    mBuffers.clear();
}

bool ScriptServer::isListening() const
{
    return mServer->isListening();
}

void ScriptServer::newConnection()
{
    while (QLocalSocket *socket = mServer->nextPendingConnection()) {
        connect(socket, &QLocalSocket::readyRead, this, [this, socket] { readyRead(socket); });
        connect(socket, &QLocalSocket::disconnected, this, [this, socket] {
            mBuffers.remove(socket);
            socket->deleteLater();
        });
    }
}

void ScriptServer::readyRead(QLocalSocket *socket)
{
    QByteArray &buffer = mBuffers[socket];
    buffer.append(socket->readAll());

    int newline;
    while ((newline = buffer.indexOf('\n')) != -1) {
        const QByteArray request = buffer.left(newline);
        buffer.remove(0, newline + 1);

        if (!request.trimmed().isEmpty())
            handleRequest(socket, request);

        // The socket may have been deleted while evaluating the script
        if (!mBuffers.contains(socket))
            return;
    }
}

void ScriptServer::handleRequest(QLocalSocket *socket, const QByteArray &request)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(request, &parseError);

    ScriptManager::EvaluationResult result;

    if (parseError.error != QJsonParseError::NoError) {
        result.error = tr("Invalid request: %1").arg(parseError.errorString());
    } else if (mBusy) {
        result.error = tr("Script server is busy evaluating another script");
    } else {
        const QJsonObject object = document.object();
        const QString script = object.value(QLatin1String("script")).toString();
        const QString fileName = object.value(QLatin1String("fileName")).toString();

        mBusy = true;
        emit scriptReceived(script);
        result = ScriptManager::instance().evaluateCaptured(script, fileName);
        emit scriptEvaluated(script, result);
        mBusy = false;
    }

    socket->write(QJsonDocument(toJson(result)).toJson(QJsonDocument::Compact));
    socket->write("\n");
    socket->flush();
}

std::optional<ScriptManager::EvaluationResult>
ScriptServer::evaluateRemotely(const QString &script,
                               const QString &fileName,
                               QString *errorMessage)
{
    QLocalSocket socket;
    socket.connectToServer(serverName());

    if (!socket.waitForConnected(500)) {
        if (errorMessage)
            *errorMessage = socket.errorString();
        return std::nullopt;
    }

    QJsonObject request { { QStringLiteral("script"), script } };
    if (!fileName.isEmpty())
        request.insert(QStringLiteral("fileName"), fileName);

    socket.write(QJsonDocument(request).toJson(QJsonDocument::Compact));
    socket.write("\n");
    if (!socket.waitForBytesWritten(5000)) {
        if (errorMessage)
            *errorMessage = socket.errorString();
        return std::nullopt;
    }

    // Wait indefinitely, since the script may take a while or wait for user
    // interaction
    QByteArray response;
    while (!response.contains('\n')) {
        if (!socket.waitForReadyRead(-1)) {
            if (errorMessage)
                *errorMessage = socket.errorString();
            return std::nullopt;
        }
        response.append(socket.readAll());
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(response.left(response.indexOf('\n')),
                                                           &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage)
            *errorMessage = tr("Invalid response: %1").arg(parseError.errorString());
        return std::nullopt;
    }

    return fromJson(document.object());
}

#include "moc_scriptserver.cpp"
