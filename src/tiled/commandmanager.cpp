/*
 * commandmanagar.cpp
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

#include <QAction>
#include <QLatin1String>
#include <QMenu>
#include <QShortcut>
#include <QSignalMapper>
#include <QWidget>

using namespace Tiled;
using namespace Tiled::Internal;

CommandManager::CommandManager(QObject *parent, QWidget *window)
	: QObject(parent)
	, mMainWindow(window)
{
	
}

CommandManager::~CommandManager()
{
}

void CommandManager::setMainWindowMenu(QMenu *menu)
{
	this->mMainWindowMenu = menu;
}

void CommandManager::populateMainWindowMenu()
{
	populateMenu(this->mMainWindowMenu);
}

void CommandManager::showDialog()
{
	CommandDialog dialog(mMainWindow);
    dialog.exec();
}

void CommandManager::populateMenu(QMenu *menu)
{
	menu->clear();

	// Add Edit Commands first
	QAction *mEditCommands = new QAction(this);
    mEditCommands->setIcon(
            QIcon(QLatin1String(":/images/24x24/system-run.png")));
    mEditCommands->setText(tr("Edit Commands"));

    menu->addAction(mEditCommands);

    connect(mEditCommands, SIGNAL(triggered()), this, SLOT(showDialog()));

    menu->addSeparator();

    // Add all enabled commands now

    bool firstEnabledCommand = true;
    int counter = -1;

    CommandDataModel *mModel = new CommandDataModel;
    const QList<Command> &commands = mModel->allCommands();

    foreach (const Command &command, commands) {
    	++counter;

        if (!command.isEnabled)
            continue;

        QAction *mAction = menu->addAction(command.name);

        firstEnabledCommand = false;

        QSignalMapper *mapper = new QSignalMapper(mAction);
        mapper->setMapping(mAction, counter);
        connect(mAction, SIGNAL(triggered()), mapper, SLOT(map()));
        connect(mapper, SIGNAL(mapped(int)), mModel, SLOT(execute(int)));

        if(firstEnabledCommand) {
        	mAction->setShortcut(QKeySequence(tr("F5")));
        	QShortcut *key = new QShortcut(QKeySequence(tr("F5")), mMainWindow);
		    connect(key, SIGNAL(activated()), mAction, SLOT(trigger()));
        }
    }
}