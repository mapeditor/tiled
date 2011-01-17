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
#include "documentmanager.h"
#include "mapdocument.h"
#include "mainwindow.h"
#include "utils.h"

#include <QSettings>
#include <QMouseEvent>
#include <QMenu>
#include <QSignalMapper>
#include <QMessageBox>
#include <QProcess>

using namespace Tiled;
using namespace Tiled::Utils;
using namespace Tiled::Internal;

CommandButton::CommandButton(MainWindow *mainWindow,
                             DocumentManager *documentManager)
    : QToolButton(mainWindow)
    , mMainWindow(mainWindow)
    , mDocumentManager(documentManager)
    , mMenu(new QMenu(this))
{
    setIcon(QIcon(QLatin1String(":images/24x24/system-run.png")));
    setThemeIcon(this, "system-run");
    setToolTip(tr("Execute Command"));
    setShortcut(QKeySequence(tr("F5")));

    setPopupMode(QToolButton::MenuButtonPopup);
    setMenu(mMenu);

    connect(mMenu, SIGNAL(aboutToShow()), SLOT(populateMenu()));
    connect(this, SIGNAL(clicked()), SLOT(runPrimaryCommand()));
}

void CommandButton::runCommand(const QString &command)
{
    // Save if save option is unset or true
    QSettings settings;
    QVariant variant = settings.value(QLatin1String("saveBeforeExecute"), true);
    if (variant.toBool())
        mMainWindow->saveFile();

    QString finalCommand = command;

    // Perform map filename replacement
    MapDocument *mapDocument = mDocumentManager->currentDocument();
    if (mapDocument) {
        const QString fileName = mapDocument->fileName();
        finalCommand.replace(QLatin1String("%mapfile"),
                             QString(QLatin1String("\"%1\"")).arg(fileName));
    }

    QProcess *process = new QProcess(window());
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(showError()));
    process->start(finalCommand);
}

void CommandButton::runPrimaryCommand()
{
    const CommandDataModel model;
    Command command = model.firstEnabledCommand();
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

    runCommand(command.command);
}

void CommandButton::showDialog()
{
    CommandDialog dialog(window());
    dialog.exec();
}

void CommandButton::populateMenu()
{
    mMenu->clear();
    QSignalMapper *mapper = new QSignalMapper(this);

    // Use a data model for getting the command list to avoid having to
    // manually parse the settings
    const CommandDataModel model;
    const QList<Command> &commands = model.allCommands();

    foreach (const Command &command, commands) {
        if (!command.isEnabled)
            continue;

        QAction *action = new QAction(command.name, this);
        action->setStatusTip(command.command);

        mapper->setMapping(action, command.command);
        connect(action, SIGNAL(triggered()), mapper, SLOT(map()));

        mMenu->addAction(action);
    }

    connect(mapper, SIGNAL(mapped(const QString &)),
                    SLOT(runCommand(const QString &)));

    if (!mMenu->isEmpty())
        mMenu->addSeparator();

    // Add "Edit Commands..." action
    QAction *action = new QAction(tr("Edit Commands..."), this);
    connect(action, SIGNAL(triggered()), SLOT(showDialog()));
    mMenu->addAction(action);
}

void CommandButton::showError()
{
    QMessageBox::warning(window(),
                         tr("Error Executing Command"),
                         tr("There was an error running the command"));
}
