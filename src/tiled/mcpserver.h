/*
 * mcpserver.h
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

#include "tilededitor_global.h"

#include <QHttpServer>
#include <QTcpServer>

class QJsonObject;
class QWidget;

namespace Tiled {

class TILED_EDITOR_EXPORT McpServer : public QObject
{
    Q_OBJECT

public:
    explicit McpServer(QObject *parent = nullptr);

    bool start();
    void stop();
    QString url() const;
    QByteArray token() const { return mToken; }
    void showDialog(QWidget *parent);

private:
    QHttpServerResponse handleRequest(const QHttpServerRequest &request);
    QJsonObject dispatch(const QJsonObject &request);
    QJsonObject executeScript(const QString &code);

    QHttpServer mHttpServer;
    QTcpServer mListener;
    QByteArray mToken;
    bool mBusy = false;
};

} // namespace Tiled
