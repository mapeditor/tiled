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

void Command::execute() const
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
    new CommandProcess(*this);
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
    QHash<QString, QVariant> hash = variant.toHash();

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

CommandProcess::CommandProcess(const Command &command)
    : QProcess(DocumentManager::instance())
    , mName(command.name)
    , mFinalCommand(command.finalCommand())
{
    // Give an error if the command is empty or just whitespace
    if(mFinalCommand.count(QLatin1String(" ")) == mFinalCommand.size()) {
        handleError(QProcess::FailedToStart);
        return;
    }

    start(mFinalCommand);

    connect(this, SIGNAL(error(QProcess::ProcessError)),
            SLOT(handleError(QProcess::ProcessError)));

    connect(this, SIGNAL(finished(int)), SLOT(deleteLater()));
}

void CommandProcess::handleError(QProcess::ProcessError error)
{
    QString errorStr;
    switch (error) {
    case QProcess::FailedToStart: errorStr = tr("The command failed to start.");
                                  break;
    case QProcess::Crashed: errorStr = tr("The command crashed."); break;
    case QProcess::Timedout: errorStr = tr("The command timed out."); break;
    default: errorStr = tr("An unknown error occurred.");
    }

    QString title = tr("Error Executing %1").arg(mName);

    QString message = errorStr + QLatin1String("\n\n") + mFinalCommand;

    QWidget *parent = (QWidget*)DocumentManager::instance()->widget();
    QMessageBox::warning(parent, title, message);

    // Make sure this object gets deleted if the process failed to start
    deleteLater();
}
