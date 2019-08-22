/*
 * newsbutton.cpp
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

#include "newsbutton.h"

#include "newsfeed.h"
#include "preferences.h"
#include "utils.h"

#include <QDesktopServices>
#include <QMenu>
#include <QPainter>

#ifdef TILED_SNAPSHOT
static const char newsArchiveUrl[] = "https://thorbjorn.itch.io/tiled/devlog";
#else
static const char newsArchiveUrl[] = "https://www.mapeditor.org/news";
#endif

namespace Tiled {

NewsButton::NewsButton(QWidget *parent)
    : QToolButton(parent)
    , mReadIcon(QLatin1String("://images/16/mail-read-symbolic.png"))
    , mUnreadIcon(QLatin1String("://images/16/mail-unread-symbolic.png"))
{
    const auto preferences = Preferences::instance();
    setVisible(preferences->displayNews());
    connect(preferences, &Preferences::displayNewsChanged, this, &NewsButton::setVisible);

    auto &feed = NewsFeed::instance();

    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setAutoRaise(true);
#ifdef TILED_SNAPSHOT
    setText(tr("Devlog"));
#else
    setText(tr("News"));
#endif
    setToolTip(feed.errorString());

    connect(&feed, &NewsFeed::refreshed,
            this, &NewsButton::refreshButton);
    connect(&feed, &NewsFeed::errorStringChanged,
            this, &NewsButton::setToolTip);

    connect(this, &QToolButton::pressed,
            this, &NewsButton::showNewsMenu);

    refreshButton();
}

void NewsButton::refreshButton()
{
    auto &feed = NewsFeed::instance();
    auto unreadCount = feed.unreadCount();

    if (unreadCount > 0) {
        QPixmap numberPixmap(Utils::smallIconSize());
        numberPixmap.fill(Qt::transparent);

        QPainter painter(&numberPixmap);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        painter.setBrush(QColor(250, 92, 92));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(numberPixmap.rect().adjusted(1, 1, -1, -1));

        auto font = painter.font();
        font.setBold(true);
        painter.setFont(font);
        painter.setBrush(Qt::white);
        painter.setPen(Qt::white);
        painter.drawText(numberPixmap.rect(), Qt::AlignCenter, unreadCount < 5 ? QString::number(unreadCount) :
                                                                                 QString(QLatin1String("!")));

        setIcon(QIcon(numberPixmap));
    } else {
        setIcon(QIcon());
    }

    setEnabled(!feed.isEmpty());
}

void NewsButton::showNewsMenu()
{
    auto newsFeedMenu = new QMenu;
    auto &feed = NewsFeed::instance();

    for (const NewsItem &newsItem : feed.newsItems()) {
        QAction *action = newsFeedMenu->addAction(newsItem.title);

        if (feed.isUnread(newsItem)) {
            QFont f = action->font();
            f.setBold(true);
            action->setFont(f);
            action->setIcon(mUnreadIcon);
        } else {
            action->setIcon(mReadIcon);
        }

        connect(action, &QAction::triggered, [=] {
            QDesktopServices::openUrl(newsItem.link);
            NewsFeed::instance().markRead(newsItem);
        });
    }

    newsFeedMenu->addSeparator();
#ifdef TILED_SNAPSHOT
    QAction *action = newsFeedMenu->addAction(tr("View All Posts"));
#else
    QAction *action = newsFeedMenu->addAction(tr("News Archive"));
#endif
    connect(action, &QAction::triggered, [] {
        QDesktopServices::openUrl(QUrl(QLatin1String(newsArchiveUrl)));
        NewsFeed::instance().markAllRead();
    });

    auto size = newsFeedMenu->sizeHint();
    auto rect = QRect(mapToGlobal(QPoint(width() - size.width(), -size.height())), size);
    newsFeedMenu->setGeometry(rect);
    newsFeedMenu->exec();

    setDown(false);
}

} // namespace Tiled
