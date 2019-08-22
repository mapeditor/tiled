/*
 * newversionchecker.h
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

#pragma once

#include <QBasicTimer>
#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace Tiled {

class NewVersionChecker : public QObject
{
    Q_OBJECT

    NewVersionChecker();

public:
    struct VersionInfo {
        QString version;
        QUrl releaseNotesUrl;
        QUrl downloadUrl;
    };

    static NewVersionChecker &instance();

    void setEnabled(bool enabled);

    void refresh();

    bool isNewVersionAvailable() const;
    const VersionInfo &versionInfo() const;
    QString errorString() const;

protected:
    void timerEvent(QTimerEvent *event) override;

signals:
    void newVersionAvailable(const VersionInfo &versionInfo);
    void errorStringChanged(const QString &errorString);

private:
    void finished(QNetworkReply *reply);

    QNetworkAccessManager *mNetworkAccessManager;
    QBasicTimer mRefreshTimer;
    QString mErrorString;
    VersionInfo mVersionInfo;
};


inline const NewVersionChecker::VersionInfo &NewVersionChecker::versionInfo() const
{
    return mVersionInfo;
}

inline QString NewVersionChecker::errorString() const
{
    return mErrorString;
}

} // namespace Tiled
