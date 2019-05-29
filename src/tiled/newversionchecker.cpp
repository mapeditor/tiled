/*
 * newversionchecker.cpp
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

#include "newversionchecker.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

static const char versionInfoUrl[] = "https://www.mapeditor.org/versions.json";

static bool versionLessThan(const QString &a, const QString &b)
{
    const QVector<QStringRef> aParts = a.splitRef(QLatin1Char('.'));
    const QVector<QStringRef> bParts = b.splitRef(QLatin1Char('.'));
    const int commonLength = std::min(aParts.size(), bParts.size());

    for (int i = 0; i < commonLength; ++i) {
        const int aNumber = aParts.at(i).toInt();
        const int bNumber = bParts.at(i).toInt();
        if (aNumber < bNumber)
            return true;
        if (bNumber < aNumber)
            return false;
    }

    // Version was the same so far, making a older when it has less parts
    return aParts.size() < bParts.size();
}

namespace Tiled {

NewVersionChecker::NewVersionChecker()
    : mNetworkAccessManager(new QNetworkAccessManager(this))
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished,
            this, &NewVersionChecker::finished);

    refresh();

    // Check for new version once every 6 hours
    auto second = 1000;
    auto minute = 60 * second;
    auto hour = 60 * minute;
    mRefreshTimer.start(6 * hour, Qt::VeryCoarseTimer, this);
}

NewVersionChecker &NewVersionChecker::instance()
{
    static NewVersionChecker instance;
    return instance;
}

/**
 * Requests the latest version from the network.
 *
 * Normally does not need to be called. The version is refreshed automatically
 * on startup and every 6 hours.
 */
void NewVersionChecker::refresh()
{
    mNetworkAccessManager->get(QNetworkRequest(QUrl(QLatin1String(versionInfoUrl))));
    mErrorString.clear();
    emit errorStringChanged(mErrorString);
}

bool NewVersionChecker::isNewVersionAvailable() const
{
    return versionLessThan(QCoreApplication::applicationVersion(), mVersionInfo.version);
}

void NewVersionChecker::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mRefreshTimer.timerId()) {
        refresh();
        return;
    }

    QObject::timerEvent(event);
}

void NewVersionChecker::finished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        mErrorString = reply->errorString();
        emit errorStringChanged(mErrorString);
        emit refreshed();
        return;
    }

    QJsonParseError error;
    QJsonObject object = QJsonDocument::fromJson(reply->readAll(), &error).object();

    if (error.error != QJsonParseError::NoError || object.isEmpty()) {
        mErrorString = error.errorString();
        emit errorStringChanged(mErrorString);
        emit refreshed();
        return;
    }

#ifdef TILED_SNAPSHOT
    QJsonObject versionObject = object.value(QStringLiteral("snapshot")).toObject();
#else
    QJsonObject versionObject = object.value(QStringLiteral("release")).toObject();
#endif

    mVersionInfo.version = versionObject.value(QStringLiteral("version")).toString();
    mVersionInfo.releaseNotesUrl = QUrl(versionObject.value(QStringLiteral("release_notes")).toString(), QUrl::StrictMode);
    mVersionInfo.downloadUrl = QUrl(versionObject.value(QStringLiteral("download")).toString(), QUrl::StrictMode);

    if (isNewVersionAvailable()) {
        emit newVersionAvailable(mVersionInfo);
        mRefreshTimer.stop();
    }

    emit refreshed();
}

} // namespace Tiled
