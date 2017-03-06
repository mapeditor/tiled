/*
 * commandmanager.cpp
 * Copyright 2017, Ketan Gupta <ketan19972010@gmail.com>
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

#include "commandmanager.h"

#include "commanddatamodel.h"
#include "commanddialog.h"

#include <QApplication>
#include <QAction>
#include <QLatin1String>
#include <QMenu>
#include <QSignalMapper>
#include <QWidget>

using namespace Tiled;
using namespace Tiled::Internal;

CommandManager::CommandManager(QObject *parent)
    : QObject(parent)
{
    
}

void CommandManager::showDialog()
{
    CommandDialog dialog(QApplication::activeWindow());
    dialog.exec();
}

void CommandManager::populateMenu(QMenu *menu)
{
    menu->clear();

    // Add all enabled commands

    bool firstEnabledCommand = true;
    int counter = -1;

    CommandDataModel *mModel = new CommandDataModel;
    const QList<Command> &commands = mModel->allCommands();

    foreach (const Command &command, commands) {
        ++counter;

        if (!command.isEnabled)
            continue;

        QAction *mAction = menu->addAction(command.name);
        if (firstEnabledCommand)
            mAction->setShortcut(QKeySequence(tr("F5")));
        firstEnabledCommand = false;

        QSignalMapper *mapper = new QSignalMapper(mAction);
        mapper->setMapping(mAction, counter);
        connect(mAction, SIGNAL(triggered()), mapper, SLOT(map()));
        connect(mapper, SIGNAL(mapped(int)), mModel, SLOT(execute(int)));
    }

    // Add Edit Commands action
    menu->addSeparator();

    QAction *mEditCommands = new QAction(this);
    mEditCommands->setIcon(
            QIcon(QLatin1String(":/images/24x24/system-run.png")));
    mEditCommands->setText(tr("Edit Commands..."));

    menu->addAction(mEditCommands);

    connect(mEditCommands, SIGNAL(triggered()), this, SLOT(showDialog()));

}
