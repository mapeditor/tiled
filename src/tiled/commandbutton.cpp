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
#include "commanddatamodel.h"
#include "commanddialog.h"
#include "utils.h"

#include <QEvent>
#include <QMenu>
#include <QMessageBox>

using namespace Tiled;
using namespace Tiled::Utils;
using namespace Tiled::Internal;

CommandButton::CommandButton(QWidget *parent)
    : QToolButton(parent)
    , mMenu(new QMenu(this))
{
    setIcon(QIcon(QLatin1String(":images/24x24/system-run.png")));
    setThemeIcon(this, "system-run");
    retranslateUi();

    setPopupMode(QToolButton::MenuButtonPopup);
    setMenu(mMenu);

    connect(mMenu, SIGNAL(aboutToShow()), SLOT(populateMenu()));
    connect(this, SIGNAL(clicked()), SLOT(runCommand()));
}

void CommandButton::runCommand()
{
    Command command;

    QAction *action = dynamic_cast<QAction*>(sender());
    if (action && action->data().isValid()) {
        //run the command passed by the action
        command = Command::fromQVariant(action->data());
    } else {
        //run the default command
        command = CommandDataModel().firstEnabledCommand();

        if (!command.isEnabled) {
            QMessageBox msgBox(window());
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowTitle(tr("Error Executing Command"));
            msgBox.setText(tr("You do not have any commands setup."));
            msgBox.addButton(QMessageBox::Ok);
            msgBox.addButton(tr("Edit commands..."), QMessageBox::ActionRole);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setEscapeButton(QMessageBox::Ok);

            QAbstractButton *button = msgBox.buttons().last();
            connect(button, SIGNAL(clicked()), SLOT(showDialog()));

            msgBox.exec();
            return;
        }
    }

    command.execute();
}

void CommandButton::showDialog()
{
    CommandDialog dialog(window());
    dialog.exec();
}

void CommandButton::populateMenu()
{
    mMenu->clear();

    // Use a data model for getting the command list to avoid having to
    // manually parse the settings
    const CommandDataModel model;
    const QList<Command> &commands = model.allCommands();

    foreach (const Command &command, commands) {
        if (!command.isEnabled)
            continue;

        QAction *action = mMenu->addAction(command.name);
        action->setStatusTip(command.command);
        action->setData(command.toQVariant());
        connect(action, SIGNAL(triggered()), SLOT(runCommand()));
    }

    if (!mMenu->isEmpty())
        mMenu->addSeparator();

    // Add "Edit Commands..." action
    QAction *action = mMenu->addAction(tr("Edit Commands..."));
    connect(action, SIGNAL(triggered()), SLOT(showDialog()));
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
    setShortcut(QKeySequence(tr("F5")));
}
