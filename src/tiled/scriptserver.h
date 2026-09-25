/*
 * scriptserver.h
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

#pragma once

#include "scriptmanager.h"
#include "tilededitor_global.h"

#include <QHash>
#include <QObject>

#include <optional>

class QLocalServer;
class QLocalSocket;

namespace Tiled {

/**
 * Local socket server that allows other processes to evaluate scripts in
 * this Tiled instance. Used by the "tiled --eval" command-line option.
 *
 * The protocol is newline-delimited JSON. Each request is an object with a
 * "script" property and an optional "fileName" property. Each response is
 * an object with the following properties:
 *
 *  - "result": string representation of the result (absent when undefined)
 *  - "tempName": global name assigned to the result (absent when undefined)
 *  - "error": error message (absent when the script evaluated successfully)
 *  - "output": array of { "type": "info"|"warning"|"error", "text": ... }
 */
class TILED_EDITOR_EXPORT ScriptServer : public QObject
{
    Q_OBJECT

public:
    explicit ScriptServer(QObject *parent = nullptr);
    ~ScriptServer() override;

    bool listen();
    void close();
    bool isListening() const;

    static QString serverName();

    /**
     * Whether the script server was requested on the command-line, in which
     * case it is started regardless of the preference.
     */
    static bool forceEnabled;

    /**
     * Evaluates the given script in a running Tiled instance. Returns nothing
     * when no running instance could be reached, in which case an error
     * message is set.
     */
    static std::optional<ScriptManager::EvaluationResult>
    evaluateRemotely(const QString &script,
                     const QString &fileName,
                     QString *errorMessage = nullptr);

signals:
    void scriptReceived(const QString &script);
    void scriptEvaluated(const QString &script,
                         const Tiled::ScriptManager::EvaluationResult &result);

private:
    void newConnection();
    void readyRead(QLocalSocket *socket);
    void handleRequest(QLocalSocket *socket, const QByteArray &request);

    QLocalServer *mServer;
    QHash<QLocalSocket*, QByteArray> mBuffers;
    bool mBusy = false;
};

} // namespace Tiled
