/*
 * command.cpp
 * Copyright 2011, Jeff Bland <jksb@member.fsf.org>
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

#include "command.h"
#include "documentmanager.h"
#include "mapdocument.h"

#include <QMessageBox>
#include <QSettings>

using namespace Tiled;
using namespace Tiled::Internal;

QString Command::finalCommand() const
{
    QString finalCommand = command;

    // Perform map filename replacement
    MapDocument *mapDocument = DocumentManager::instance()->currentDocument();
    if (mapDocument) {
        const QString fileName = mapDocument->fileName();
        finalCommand.replace(QLatin1String("%mapfile"),
                             QString(QLatin1String("\"%1\"")).arg(fileName));
    }

    return finalCommand;
}

void Command::execute(bool inTerminal) const
{
    // Save if save option is unset or true
    QSettings settings;
    QVariant variant = settings.value(QLatin1String("saveBeforeExecute"), true);
    if (variant.toBool()) {
        MapDocument *document = DocumentManager::instance()->currentDocument();
        if (document)
            document->save();
    }

    // Start the process
    new CommandProcess(*this, inTerminal);
}

QVariant Command::toQVariant() const
{
    QHash<QString, QVariant> hash;
    hash[QLatin1String("Enabled")] = isEnabled;
    hash[QLatin1String("Name")] = name;
    hash[QLatin1String("Command")] = command;
    return hash;
}

Command Command::fromQVariant(const QVariant &variant)
{
    const QHash<QString, QVariant> hash = variant.toHash();

    const QString namePref = QLatin1String("Name");
    const QString commandPref = QLatin1String("Command");
    const QString enablePref = QLatin1String("Enabled");

    Command command;
    if (hash.contains(enablePref))
        command.isEnabled = hash[enablePref].toBool();
    if (hash.contains(namePref))
        command.name = hash[namePref].toString();
    if (hash.contains(commandPref))
        command.command = hash[commandPref].toString();

    return command;
}

CommandProcess::CommandProcess(const Command &command, bool inTerminal)
    : QProcess(DocumentManager::instance())
    , mName(command.name)
    , mFinalCommand(command.finalCommand())
#ifdef Q_WS_MAC
    , mFile(QLatin1String("tiledXXXXXX.command"))
#endif
{
    // Give an error if the command is empty or just whitespace
    if (mFinalCommand.trimmed().isEmpty()) {
        handleError(QProcess::FailedToStart);
        return;
    }

    // Modify the command to run in a terminal
    if (inTerminal) {
#ifdef Q_WS_X11
        static bool hasGnomeTerminal = QProcess::execute(
                                    QLatin1String("which gnome-terminal")) == 0;

        if (hasGnomeTerminal)
            mFinalCommand = QLatin1String("gnome-terminal -x ") + mFinalCommand;
        else
            mFinalCommand = QLatin1String("xterm -e ") + mFinalCommand;
#elif defined(Q_WS_MAC)
        // The only way I know to launch a Terminal with a command on mac is
        // to make a .command file and open it. The client command invoke the
        // exectuable directly (rather than using open) in order to get std
        // output in the terminal. Otherwise, you can use the Console
        // application to see the output.

        // Create and write the command to a .command file

        if (!mFile.open()) {
            handleError(tr("Unable to create/open %1").arg(mFile.fileName()));
            return;
        }
        mFile.write(mFinalCommand.toStdString().c_str());
        mFile.close();

        // Add execute permission to the file
        int chmodRet = QProcess::execute(QString(QLatin1String(
                                     "chmod +x \"%1\"")).arg(mFile.fileName()));
        if (chmodRet != 0) {
            handleError(tr("Unable to add executable permissions to %1")
                                                        .arg(mFile.fileName()));
            return;
        }

        // Use open command to launch the command in the terminal
        // -W makes it not return immediately
        // -n makes it open a new instance of terminal if it is open already
        mFinalCommand = QString(QLatin1String("open -W -n \"%1\""))
                                                         .arg(mFile.fileName());
#endif
    }

    connect(this, SIGNAL(error(QProcess::ProcessError)),
            SLOT(handleError(QProcess::ProcessError)));

    connect(this, SIGNAL(finished(int)), SLOT(deleteLater()));

    start(mFinalCommand);
}

void CommandProcess::handleError(QProcess::ProcessError error)
{
    QString errorStr;
    switch (error) {
    case QProcess::FailedToStart:
        errorStr = tr("The command failed to start.");
        break;
    case QProcess::Crashed:
        errorStr = tr("The command crashed.");
        break;
    case QProcess::Timedout:
        errorStr = tr("The command timed out.");
        break;
    default:
        errorStr = tr("An unknown error occurred.");
    }

    handleError(errorStr);
}

void CommandProcess::handleError(const QString &error)
{
    QString title = tr("Error Executing %1").arg(mName);

    QString message = error + QLatin1String("\n\n") + mFinalCommand;

    QWidget *parent = DocumentManager::instance()->widget();
    QMessageBox::warning(parent, title, message);

    // Make sure this object gets deleted if the process failed to start
    deleteLater();
}
