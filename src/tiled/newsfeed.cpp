/*
 * newsfeed.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "newsfeed.h"

#include "preferences.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTimerEvent>
#include <QXmlStreamReader>

#ifdef TILED_SNAPSHOT
static const char feedUrl[] = "https://thorbjorn.itch.io/tiled/devlog.rss";
#else
static const char feedUrl[] = "https://www.mapeditor.org/rss.xml";
#endif

namespace Tiled {

NewsFeed::NewsFeed()
    : mNetworkAccessManager(new QNetworkAccessManager(this))
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished,
            this, &NewsFeed::finished);

    const auto preferences = Preferences::instance();
    const auto settings = preferences->settings();
    mLastRead = settings->value(QLatin1String("Install/NewsFeedLastRead")).toDateTime();

    setEnabled(preferences->displayNews());
    connect(preferences, &Preferences::displayNewsChanged, this, &NewsFeed::setEnabled);
}

NewsFeed &NewsFeed::instance()
{
    static NewsFeed newsFeed;
    return newsFeed;
}

void NewsFeed::setEnabled(bool enabled)
{
    if (mRefreshTimer.isActive() == enabled)
        return;

    if (enabled) {
        refresh();

        // Refresh the news feed once every 4 hours
        auto second = 1000;
        auto minute = 60 * second;
        auto hour = 60 * minute;
        mRefreshTimer.start(4 * hour, Qt::VeryCoarseTimer, this);
    } else {
        mRefreshTimer.stop();
    }
}

/**
 * Requests the feed from the network.
 *
 * Can be called to request a news feed update when automatic refresh
 * has been disabled.
 */
void NewsFeed::refresh()
{
    mNetworkAccessManager->get(QNetworkRequest(QUrl(QLatin1String(feedUrl))));
    mErrorString.clear();
    emit errorStringChanged(mErrorString);
}

void NewsFeed::markAllRead()
{
    if (mNewsItems.isEmpty())
        return;

    markRead(mNewsItems.first());
}

/**
 * Marks all items up to the given \a item as read.
 */
void NewsFeed::markRead(const NewsItem &item)
{
    if (mLastRead < item.pubDate)
        setLastRead(item.pubDate);
}

bool NewsFeed::isUnread(const NewsItem &item) const
{
    return item.pubDate > mLastRead;
}

int NewsFeed::unreadCount() const
{
    int count = 0;
    for (const NewsItem &item : mNewsItems)
        if (isUnread(item))
            ++count;
    return count;
}

void NewsFeed::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mRefreshTimer.timerId()) {
        refresh();
        return;
    }

    QObject::timerEvent(event);
}

void NewsFeed::finished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        mErrorString = reply->errorString();
        qWarning() << mErrorString;
        emit errorStringChanged(mErrorString);
        return;
    }

    QXmlStreamReader xml(reply);

    if (!xml.readNextStartElement() || xml.name() != QLatin1String("rss"))
        return;
    if (!xml.readNextStartElement() || xml.name() != QLatin1String("channel"))
        return;

    mNewsItems.clear();

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("item")) {
            NewsItem newsItem;

            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("title"))
                    newsItem.title = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                else if (xml.name() == QLatin1String("link"))
                    newsItem.link = QUrl(xml.readElementText(QXmlStreamReader::SkipChildElements));
                else if (xml.name() == QLatin1String("pubDate"))
                    newsItem.pubDate = QDateTime::fromString(xml.readElementText(QXmlStreamReader::SkipChildElements), Qt::RFC2822Date);
                else
                    xml.skipCurrentElement();
            }

            mNewsItems.append(newsItem);

            // No need for parsing everything
            if (mNewsItems.size() == 5)
                break;

        } else {
            xml.skipCurrentElement();
        }
    }

    if (xml.hasError())
        qWarning() << xml.errorString();

    emit refreshed();
}

void NewsFeed::setLastRead(const QDateTime &dateTime)
{
    mLastRead = dateTime;

    auto settings = Preferences::instance()->settings();
    settings->setValue(QLatin1String("Install/NewsFeedLastRead"),
                       mLastRead.toString(Qt::ISODate));

    emit refreshed();
}

} // namespace Tiled
