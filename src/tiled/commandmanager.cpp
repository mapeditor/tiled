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
#include "utils.h"

#include <QApplication>
#include <QAction>
#include <QLatin1String>
#include <QMenu>

namespace Tiled {
namespace Internal {

CommandManager *CommandManager::mInstance;

CommandManager::CommandManager()
    : mModel(new CommandDataModel(this))
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

    const QList<Command> &commands = mModel->allCommands();

    for (int i = 0; i < commands.size(); ++i) {
        const Command &command = commands.at(i);

        if (!command.isEnabled)
            continue;

        QAction *mAction = menu->addAction(command.name);
        if (firstEnabledCommand)
            mAction->setShortcut(QKeySequence(tr("F5")));
        firstEnabledCommand = false;

        connect(mAction, &QAction::triggered, [this,i]() { mModel->execute(i); });
    }

    // Add Edit Commands action
    if (!menu->isEmpty())
        menu->addSeparator();

    QAction *mEditCommands = new QAction(this);
    mEditCommands->setIcon(
            QIcon(QLatin1String(":/images/24x24/system-run.png")));
    mEditCommands->setText(tr("Edit Commands..."));
    Utils::setThemeIcon(mEditCommands, "system-run");

    menu->addAction(mEditCommands);

    connect(mEditCommands, &QAction::triggered, this, &CommandManager::showDialog);

}

} // namespace Internal
} // namespace Tiled
