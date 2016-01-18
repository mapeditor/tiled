/*
 * autoupdater.h
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

#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H

#include <QString>

class AutoUpdater
{
public:
    AutoUpdater();
    virtual ~AutoUpdater();

    virtual void checkForUpdates() = 0;

    virtual void setAutomaticallyChecksForUpdates(bool on) = 0;
    virtual bool automaticallyChecksForUpdates() = 0;

    virtual QString lastUpdateCheckDate() = 0;

    static AutoUpdater *instance() { return sInstance; }

private:
    static AutoUpdater *sInstance;
};

#endif // AUTOUPDATER_H
