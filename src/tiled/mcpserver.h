/*
 * This file is part of Tiled, licensed under the GNU GPL, version 2 or later.
 */

#pragma once

#include "tilededitor_global.h"

#include <QHttpServer>
#include <QTcpServer>

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
