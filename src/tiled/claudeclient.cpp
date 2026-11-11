/*
 * claudeclient.cpp
 *
 * This file is part of Tiled.
 * Licensed under the GNU General Public License v2 or later.
 */

#include "claudeclient.h"

#include "preferences.h"

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace Tiled {

static const char apiKeyKey[] = "Ai/ApiKey";
static const char modelKey[] = "Ai/Model";
static const char defaultModel[] = "claude-sonnet-5-5";

ClaudeClient::ClaudeClient(QObject *parent)
    : QObject(parent)
    , mNetwork(new QNetworkAccessManager(this))
{
}

QString ClaudeClient::apiKey()
{
    QString key = Preferences::instance()->value(QLatin1String(apiKeyKey)).toString();
    if (key.isEmpty())
        key = qEnvironmentVariable("ANTHROPIC_API_KEY");
    return key;
}

void ClaudeClient::setApiKey(const QString &key)
{
    Preferences::instance()->setValue(QLatin1String(apiKeyKey), key);
}

QString ClaudeClient::model()
{
    return Preferences::instance()->value(QLatin1String(modelKey),
                                          QLatin1String(defaultModel)).toString();
}

void ClaudeClient::setModel(const QString &model)
{
    Preferences::instance()->setValue(QLatin1String(modelKey), model);
}

QJsonObject ClaudeClient::textBlock(const QString &text)
{
    return QJsonObject {
        { QLatin1String("type"), QLatin1String("text") },
        { QLatin1String("text"), text }
    };
}

QJsonObject ClaudeClient::imageBlock(const QByteArray &png)
{
    return QJsonObject {
        { QLatin1String("type"), QLatin1String("image") },
        { QLatin1String("source"), QJsonObject {
              { QLatin1String("type"), QLatin1String("base64") },
              { QLatin1String("media_type"), QLatin1String("image/png") },
              { QLatin1String("data"), QString::fromLatin1(png.toBase64()) }
          } }
    };
}

void ClaudeClient::send(const QString &system,
                        const QJsonArray &userContent,
                        int maxTokens,
                        Callback callback)
{
    const QString key = apiKey();
    if (key.isEmpty()) {
        callback({ false, QString(), tr("No API key set. Enter your Claude API key in the AI panel.") });
        return;
    }

    QNetworkRequest request(QUrl(QStringLiteral("https://api.anthropic.com/v1/messages")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("x-api-key", key.toUtf8());
    request.setRawHeader("anthropic-version", "2023-06-01");
    request.setTransferTimeout(300 * 1000);

    const QJsonObject body {
        { QLatin1String("model"), model() },
        { QLatin1String("max_tokens"), maxTokens },
        { QLatin1String("system"), system },
        { QLatin1String("messages"), QJsonArray {
              QJsonObject {
                  { QLatin1String("role"), QLatin1String("user") },
                  { QLatin1String("content"), userContent }
              }
          } }
    };

    QNetworkReply *reply = mNetwork->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [reply, callback] {
        reply->deleteLater();

        const QByteArray data = reply->readAll();
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        const QJsonObject root = doc.object();

        if (reply->error() != QNetworkReply::NoError || root.value(QLatin1String("type")).toString() == QLatin1String("error")) {
            QString message = root.value(QLatin1String("error")).toObject().value(QLatin1String("message")).toString();
            if (message.isEmpty())
                message = reply->errorString();
            callback({ false, QString(), message });
            return;
        }

        QString text;
        const QJsonArray content = root.value(QLatin1String("content")).toArray();
        for (const QJsonValue &block : content) {
            const QJsonObject o = block.toObject();
            if (o.value(QLatin1String("type")).toString() == QLatin1String("text"))
                text += o.value(QLatin1String("text")).toString();
        }

        if (text.isEmpty()) {
            callback({ false, QString(), tr("Empty reply from Claude") });
            return;
        }

        if (root.value(QLatin1String("stop_reason")).toString() == QLatin1String("max_tokens")) {
            callback({ false, text, tr("The reply was cut off (too large). Try a smaller map or area.") });
            return;
        }

        callback({ true, text, QString() });
    });
}

QJsonObject ClaudeClient::extractJson(const QString &text, QString *error)
{
    const int start = text.indexOf(QLatin1Char('{'));
    const int end = text.lastIndexOf(QLatin1Char('}'));
    if (start < 0 || end <= start) {
        if (error)
            *error = tr("No JSON found in the reply");
        return {};
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(text.mid(start, end - start + 1).toUtf8(), &parseError);
    if (doc.isNull() || !doc.isObject()) {
        if (error)
            *error = tr("Invalid JSON in the reply: %1").arg(parseError.errorString());
        return {};
    }
    return doc.object();
}

} // namespace Tiled

#include "moc_claudeclient.cpp"
