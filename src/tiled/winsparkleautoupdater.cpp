/*
 * winsparkleautoupdater.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "winsparkleautoupdater.h"

#include <QCoreApplication>

#include <winsparkle.h>

namespace Tiled {
namespace Internal {

#ifdef TILED_SNAPSHOT
static const char appcastUrl[] = "http://update.mapeditor.org/appcast-win-snapshots.xml";
#else
static const char appcastUrl[] = "http://update.mapeditor.org/appcast-win.xml";
#endif


WinSparkleAutoUpdater::WinSparkleAutoUpdater()
{
    win_sparkle_set_appcast_url(appcastUrl);
    win_sparkle_init();
}

WinSparkleAutoUpdater::~WinSparkleAutoUpdater()
{
    win_sparkle_cleanup();
}

void WinSparkleAutoUpdater::checkForUpdates()
{
    win_sparkle_check_update_with_ui();
}

void WinSparkleAutoUpdater::setAutomaticallyChecksForUpdates(bool on)
{
    win_sparkle_set_automatic_check_for_updates(on ? 1 : 0);
}

bool WinSparkleAutoUpdater::automaticallyChecksForUpdates()
{
    return win_sparkle_get_automatic_check_for_updates() == 1;
}

QDateTime WinSparkleAutoUpdater::lastUpdateCheckDate()
{
    time_t lastUpdate = win_sparkle_get_last_check_time();
    if (lastUpdate == -1)
        return QDateTime(); // never checked yet

    return QDateTime::fromTime_t(lastUpdate);
}

} // namespace Internal
} // namespace Tiled
