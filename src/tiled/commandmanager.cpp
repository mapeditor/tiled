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
#include "logginginterface.h"
#include "pluginmanager.h"
#include "utils.h"

#include <QApplication>
#include <QAction>
#include <QLatin1String>
#include <QMenu>

#include "qtcompat_p.h"

namespace Tiled {

CommandManager *CommandManager::mInstance;

CommandManager::CommandManager()
    : mModel(new CommandDataModel(this))
{
    updateActions();
}

CommandManager::~CommandManager()
{
}

CommandManager *CommandManager::instance()
{
    if (!mInstance)
        mInstance = new CommandManager;

    return mInstance;
}

void CommandManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

CommandDataModel *CommandManager::commandDataModel()
{
    return mModel;
}

void CommandManager::registerMenu(QMenu *menu)
{
    mMenus.append(menu);
    menu->clear();
    menu->addActions(mActions);
}

void CommandManager::showDialog()
{
    CommandDialog dialog(QApplication::activeWindow());
    dialog.exec();
}

void CommandManager::populateMenus()
{
    for (QMenu *menu : qAsConst(mMenus)) {
        menu->clear();
        menu->addActions(mActions);
    }
}

void CommandManager::updateActions()
{
    qDeleteAll(mActions);
    mActions.clear();

    const QList<Command> &commands = mModel->allCommands();

    for (int i = 0; i < commands.size(); ++i) {
        const Command &command = commands.at(i);

        if (!command.isEnabled)
            continue;

        QAction *mAction = new QAction(command.name, this);
        mAction->setShortcut(command.shortcut);

        connect(mAction, &QAction::triggered, [this,i] { mModel->execute(i); });

        mActions.append(mAction);
    }

    // Add Edit Commands action
    QAction *mSeparator = new QAction(this);
    mSeparator->setSeparator(true);

    mActions.append(mSeparator);

    mEditCommands = new QAction(this);
    mEditCommands->setIcon(
            QIcon(QLatin1String(":/images/24/system-run.png")));
    Utils::setThemeIcon(mEditCommands, "system-run");

    connect(mEditCommands, &QAction::triggered, this, &CommandManager::showDialog);

    mActions.append(mEditCommands);

    retranslateUi();
    populateMenus();
}

void CommandManager::retranslateUi()
{
    mEditCommands->setText(tr("Edit Commands..."));
}

} // namespace Tiled
