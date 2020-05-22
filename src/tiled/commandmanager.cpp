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
#include "preferences.h"
#include "utils.h"

#include <QApplication>
#include <QAction>
#include <QLatin1String>
#include <QMenu>

#include "qtcompat_p.h"

namespace Tiled {

CommandManager::CommandManager()
    : mModel(new CommandDataModel(this))
{
    auto preferences = Preferences::instance();

    // Load command list
    const auto commands = preferences->value(QLatin1String("commandList")).toList();
    for (const QVariant &commandVariant : commands)
        mCommands.append(Command::fromQVariant(commandVariant));

    // Add default commands the first time the app has booted up.
    // This is useful on its own and helps demonstrate how to use the commands.

    Preference<bool> addedDefault { "addedDefaultCommands", false };
    if (!addedDefault) {
        // Disable default commands by default so user gets an informative
        // warning when clicking the command button for the first time
        Command command;
        command.isEnabled = false;
#ifdef Q_OS_LINUX
        command.executable = QLatin1String("gedit");
        command.arguments = QLatin1String("%mapfile");
#elif defined(Q_OS_MAC)
        command.executable = QLatin1String("open");
        command.arguments = QLatin1String("-t %mapfile");
#endif
        if (!command.executable.isEmpty()) {
            command.name = tr("Open in text editor");
            mCommands.push_back(command);
        }

        commit();
        addedDefault = true;
    }

    updateActions();
}

CommandManager::~CommandManager()
{
}

const Command *CommandManager::firstEnabledCommand() const
{
    for (const Command &command : mCommands)
        if (command.isEnabled)
            return &command;

    return nullptr;
}

CommandManager *CommandManager::instance()
{
    static CommandManager instance;
    return &instance;
}

QVector<Command> CommandManager::commands() const
{
    return mCommands;
}

/**
 * Registers a new QMenu with the CommandManager.
 */
void CommandManager::registerMenu(QMenu *menu)
{
    mMenus.append(menu);
    menu->clear();
    menu->addActions(mActions);
}

/**
 * Displays the dialog to edit the commands.
 */
void CommandManager::showDialog()
{
    CommandDialog dialog(mCommands, QApplication::activeWindow());
    dialog.exec();

    mCommands = dialog.commands();

    commit();
    updateActions();
}

/**
 * Saves the data to the users preferences.
 */
void CommandManager::commit()
{
    QVariantList commands;
    for (const Command &command : qAsConst(mCommands))
        commands.append(command.toQVariant());

    Preferences::instance()->setValue(QLatin1String("commandList"), commands);
}

void CommandManager::updateActions()
{
    qDeleteAll(mActions);
    mActions.clear();

    for (const Command &command : qAsConst(mCommands)) {
        if (!command.isEnabled)
            continue;

        QAction *mAction = new QAction(command.name, this);
        mAction->setShortcut(command.shortcut);

        connect(mAction, &QAction::triggered, [command] { command.execute(); });

        mActions.append(mAction);
    }

    // Add Edit Commands action
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    mActions.append(separator);

    mEditCommandsAction = new QAction(this);
    mEditCommandsAction->setIcon(
            QIcon(QLatin1String(":/images/24/system-run.png")));
    Utils::setThemeIcon(mEditCommandsAction, "system-run");

    connect(mEditCommandsAction, &QAction::triggered, this, &CommandManager::showDialog);

    mActions.append(mEditCommandsAction);

    retranslateUi();

    // Populate registered menus
    for (QMenu *menu : mMenus) {
        menu->clear();
        menu->addActions(mActions);
    }
}

void CommandManager::retranslateUi()
{
    mEditCommandsAction->setText(tr("Edit Commands..."));
}

} // namespace Tiled
