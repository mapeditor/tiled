/*
 * newversionbutton.cpp
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

#include "newversionbutton.h"

#include "newversiondialog.h"
#include "preferences.h"
#include "utils.h"

#include <QGuiApplication>

namespace Tiled {

NewVersionButton::NewVersionButton(QWidget *parent)
    : NewVersionButton(ManualVisible, parent)
{
}

NewVersionButton::NewVersionButton(NewVersionButton::Visibility visibility, QWidget *parent)
    : QToolButton(parent)
    , mVisiblity(visibility)
{
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setAutoRaise(true);
    setText(tr("Up to date"));
    setEnabled(false);

    const auto &checker = NewVersionChecker::instance();

    connect(&checker, &NewVersionChecker::newVersionAvailable,
            this, &NewVersionButton::newVersionAvailable);

    connect(&checker, &NewVersionChecker::errorStringChanged,
            this, &NewVersionButton::errorStringChanged);

    if (checker.isNewVersionAvailable())
        newVersionAvailable(checker.versionInfo());
    else if (visibility == AutoVisible)
        setVisible(false);
    else
        errorStringChanged(checker.errorString());

    connect(Preferences::instance(), &Preferences::checkForUpdatesChanged,
            this, &NewVersionButton::updateVisiblity);

    connect(this, &QToolButton::clicked, this, [this, &checker] {
        NewVersionDialog(checker.versionInfo(), window()).exec();
    });
}

void NewVersionButton::newVersionAvailable(const NewVersionChecker::VersionInfo &versionInfo)
{
    setIcon(QIcon(QLatin1String("://images/scalable/software-update-available-symbolic.svg")));
    setEnabled(true);
    setText(tr("Update Available"));
    setToolTip(tr("%1 %2 is available").arg(QGuiApplication::applicationDisplayName(),
                                            versionInfo.version));

    if (mVisiblity == AutoVisible)
        setVisible(Preferences::instance()->checkForUpdates());
}

void NewVersionButton::errorStringChanged(const QString &errorString)
{
    if (errorString.isEmpty())
        return;

    setToolTip(errorString);
    setIcon(QIcon());
    setEnabled(false);
    setText(tr("Error checking for updates"));
}

void NewVersionButton::updateVisiblity()
{
    if (mVisiblity != AutoVisible)
        return;

    const auto preferences = Preferences::instance();
    const auto &checker = NewVersionChecker::instance();

    setVisible(preferences->checkForUpdates() && checker.isNewVersionAvailable());
}

} // namespace Tiled
