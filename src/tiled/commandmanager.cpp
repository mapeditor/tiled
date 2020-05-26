/*
 * commandmanager.cpp
 * Copyright 2017, Ketan Gupta <ketan19972010@gmail.com>
 * Copyright 2018-2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#include "mainwindow.h"
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
        mCommands.append(Command::fromVariant(commandVariant));

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

    connect(MainWindow::instance(), &MainWindow::projectChanged,
            this, &CommandManager::updateActions);
}

CommandManager::~CommandManager()
{
}

CommandManager *CommandManager::instance()
{
    static CommandManager instance;
    return &instance;
}

bool CommandManager::executeDefaultCommand() const
{
    const auto commands = allCommands();
    for (const Command &command : commands) {
        if (command.isEnabled) {
            command.execute();
            return true;
        }
    }
    return false;
}

const QVector<Command> &CommandManager::globalCommands() const
{
    return mCommands;
}

const QVector<Command> &CommandManager::projectCommands() const
{
    auto &project = MainWindow::instance()->project();
    return project.mCommands;
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
    CommandDialog dialog(QApplication::activeWindow());
    dialog.exec();

    mCommands = dialog.globalCommands();
    commit();

    auto &project = MainWindow::instance()->project();
    project.mCommands = dialog.projectCommands();
    project.save();

    updateActions();
}

/**
 * Saves the data to the users preferences.
 */
void CommandManager::commit()
{
    QVariantList commands;
    for (const Command &command : qAsConst(mCommands))
        commands.append(command.toVariant());

    Preferences::instance()->setValue(QLatin1String("commandList"), commands);
}

void CommandManager::updateActions()
{
    qDeleteAll(mActions);
    mActions.clear();

    auto addAction = [this] (const Command &command) {
        if (!command.isEnabled)
            return;

        QAction *mAction = new QAction(command.name, this);
        mAction->setShortcut(command.shortcut);
        connect(mAction, &QAction::triggered, [command] { command.execute(); });
        mActions.append(mAction);
    };

    auto addSeparator = [this] {
        QAction *separator = new QAction(this);
        separator->setSeparator(true);
        mActions.append(separator);
    };

    // Add global commands
    for (const Command &command : qAsConst(mCommands))
        addAction(command);

    addSeparator();

    // Add project-specific commands
    const auto &project = MainWindow::instance()->project();
    for (const Command &command : project.mCommands)
        addAction(command);

    addSeparator();

    // Add Edit Commands action
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
