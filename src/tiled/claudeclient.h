/*
 * claudeclient.h
 *
 * Minimal client for the Anthropic Messages API, used by the AI map tools.
 *
 * This file is part of Tiled.
 * Licensed under the GNU General Public License v2 or later.
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>

#include <functional>

class QNetworkAccessManager;

namespace Tiled {

class ClaudeClient : public QObject
{
    Q_OBJECT

public:
    struct Result {
        bool ok = false;
        QString text;   // concatenated text blocks on success
        QString error;  // human readable error on failure
    };

    using Callback = std::function<void(const Result &)>;

    explicit ClaudeClient(QObject *parent = nullptr);

    // Settings live in Preferences under "Ai/ApiKey" and "Ai/Model".
    // The ANTHROPIC_API_KEY environment variable is used as a fallback.
    static QString apiKey();
    static void setApiKey(const QString &key);
    static QString model();
    static void setModel(const QString &model);

    /**
     * Sends a single user turn made of the given content blocks (text and/or
     * image blocks) and calls \a callback when the reply arrives.
     */
    void send(const QString &system,
              const QJsonArray &userContent,
              int maxTokens,
              Callback callback);

    static QJsonObject textBlock(const QString &text);
    static QJsonObject imageBlock(const QByteArray &png);

    /**
     * Extracts the first top-level JSON object from \a text, tolerating
     * markdown code fences and leading/trailing prose.
     */
    static QJsonObject extractJson(const QString &text, QString *error = nullptr);

private:
    QNetworkAccessManager *mNetwork;
};

} // namespace Tiled
