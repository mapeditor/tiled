/*
 * commandbutton.cpp
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "commandbutton.h"
#include "commanddialog.h"
#include "commandmanager.h"
#include "utils.h"

#include <QEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

using namespace Tiled;

CommandButton::CommandButton(QWidget *parent)
    : QToolButton(parent)
{
    setIcon(QIcon(QLatin1String(":images/24/system-run.png")));
    Utils::setThemeIcon(this, "system-run");
    retranslateUi();

    auto menu = new QMenu(this);
    setMenu(menu);
    setPopupMode(QToolButton::MenuButtonPopup);

    CommandManager::instance()->registerMenu(menu);

    connect(this, &QAbstractButton::clicked, this, &CommandButton::runCommand);
}

void CommandButton::runCommand()
{
    if (CommandManager::instance()->executeDefaultCommand())
        return;

    QMessageBox warning(QMessageBox::Warning,
                        tr("Error Executing Command"),
                        tr("You do not have any commands setup."),
                        QMessageBox::Ok,
                        window());

    const auto editButton = warning.addButton(tr("Edit Commands..."), QMessageBox::ActionRole);
    warning.setDefaultButton(QMessageBox::Ok);
    warning.setEscapeButton(QMessageBox::Ok);

    connect(editButton, &QAbstractButton::clicked,
            CommandManager::instance(), &CommandManager::showDialog);

    warning.exec();
}

void CommandButton::changeEvent(QEvent *event)
{
    QToolButton::changeEvent(event);

    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void CommandButton::retranslateUi()
{
    setToolTip(tr("Execute Command"));
}
