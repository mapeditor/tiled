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

#include "commandmanager.h"
#include "documentmanager.h"
#include "logginginterface.h"
#include "mapdocument.h"
#include "mapobject.h"

#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

using namespace Tiled;

QString Command::finalWorkingDirectory() const
{
    QString finalWorkingDirectory = workingDirectory;

    finalWorkingDirectory = replaceVariables(finalWorkingDirectory, false);

    QString finalExecutable = replaceVariables(executable);
    QFileInfo mFile(finalExecutable);

    if (!mFile.exists())
        mFile = QFileInfo(QStandardPaths::findExecutable(finalExecutable));

    finalWorkingDirectory.replace(QLatin1String("%executablepath"),
                                  mFile.absolutePath());

    return finalWorkingDirectory;
}

QString Command::finalCommand() const
{
    QString finalCommand = QString(QLatin1String("%1 %2")).arg(executable, arguments);

    return replaceVariables(finalCommand);
}

QString Command::replaceVariables(const QString &string, bool quoteValues) const
{
    QString finalString = string;

    QString replaceString = (quoteValues) ? QString(QLatin1String("\"%1\"")) :
                                            QString(QLatin1String("%1"));

    // Perform variable replacement
    if (Document *document = DocumentManager::instance()->currentDocument()) {
        const QString fileName = document->fileName();

        finalString.replace(QLatin1String("%mapfile"),
                            replaceString.arg(fileName));

        QFileInfo fileInfo(fileName);
        QString mapPath = fileInfo.absolutePath();

        finalString.replace(
            QLatin1String("%mappath"),
            replaceString.arg(mapPath));

        if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
            if (const Layer *layer = mapDocument->currentLayer()) {
                finalString.replace(QLatin1String("%layername"),
                                    replaceString.arg(layer->name()));
            }
        }

        if (MapObject *currentObject = dynamic_cast<MapObject *>(document->currentObject())) {
            finalString.replace(QLatin1String("%objecttype"),
                                replaceString.arg(currentObject->type()));
            finalString.replace(QLatin1String("%objectid"),
                                replaceString.arg(currentObject->id()));
        }
    }

    return finalString;
}

void Command::execute(bool inTerminal) const
{
    if (saveBeforeExecute) {
        Document *document = DocumentManager::instance()->currentDocument();
        if (document)
            document->save(document->fileName());
    }

    // Start the process
    new CommandProcess(*this, inTerminal, showOutput);
}

QVariant Command::toQVariant() const
{
    QHash<QString, QVariant> hash;
    hash[QLatin1String("Enabled")] = isEnabled;
    hash[QLatin1String("Name")] = name;
    hash[QLatin1String("Command")] = executable;
    hash[QLatin1String("Arguments")] = arguments;
    hash[QLatin1String("WorkingDirectory")] = workingDirectory;
    hash[QLatin1String("Shortcut")] = shortcut;
    hash[QLatin1String("ShowOutput")] = showOutput;
    hash[QLatin1String("SaveBeforeExecute")] = saveBeforeExecute;
    return hash;
}

Command Command::fromQVariant(const QVariant &variant)
{
    const QHash<QString, QVariant> hash = variant.toHash();

    const QString namePref = QLatin1String("Name");
    const QString executablePref = QLatin1String("Command");
    const QString argumentsPref = QLatin1String("Arguments");
    const QString workingDirectoryPref = QLatin1String("WorkingDirectory");
    const QString enablePref = QLatin1String("Enabled");
    const QString shortcutPref = QLatin1String("Shortcut");
    const QString showOutputPref = QLatin1String("ShowOutput");
    const QString saveBeforeExecutePref = QLatin1String("SaveBeforeExecute");

    Command command;
    if (hash.contains(enablePref))
        command.isEnabled = hash[enablePref].toBool();
    if (hash.contains(namePref))
        command.name = hash[namePref].toString();
    if (hash.contains(executablePref))
        command.executable = hash[executablePref].toString();
    if (hash.contains(argumentsPref))
        command.arguments = hash[argumentsPref].toString();
    if (hash.contains(workingDirectoryPref))
        command.workingDirectory = hash[workingDirectoryPref].toString();
    if (hash.contains(shortcutPref))
        command.shortcut = hash[shortcutPref].value<QKeySequence>();
    if (hash.contains(showOutputPref))
        command.showOutput = hash[showOutputPref].toBool();
    if (hash.contains(saveBeforeExecutePref))
        command.saveBeforeExecute = hash[saveBeforeExecutePref].toBool();

    return command;
}

CommandProcess::CommandProcess(const Command &command, bool inTerminal, bool showOutput)
    : QProcess(DocumentManager::instance())
    , mName(command.name)
    , mFinalCommand(command.finalCommand())
    , mFinalWorkingDirectory(command.finalWorkingDirectory())
#ifdef Q_OS_MAC
    , mFile(QDir::tempPath() + QLatin1String("/tiledXXXXXX.command"))
#endif
{
    // Give an error if the command is empty or just whitespace
    if (mFinalCommand.trimmed().isEmpty()) {
        handleProcessError(QProcess::FailedToStart);
        return;
    }

    // Modify the command to run in a terminal
    if (inTerminal) {
#ifdef Q_OS_LINUX
        static bool hasGnomeTerminal = QProcess::execute(
                                    QLatin1String("which gnome-terminal")) == 0;

        if (hasGnomeTerminal)
            mFinalCommand = QLatin1String("gnome-terminal -x ") + mFinalCommand;
        else
            mFinalCommand = QLatin1String("xterm -e ") + mFinalCommand;
#elif defined(Q_OS_MAC)
        // The only way I know to launch a Terminal with a command on mac is
        // to make a .command file and open it. The client command invoke the
        // executable directly (rather than using open) in order to get std
        // output in the terminal. Otherwise, you can use the Console
        // application to see the output.

        // Create and write the command to a .command file

        if (!mFile.open()) {
            reportErrorAndDelete(tr("Unable to create/open %1").arg(mFile.fileName()));
            return;
        }
        mFile.write(mFinalCommand.toLocal8Bit());
        mFile.close();

        // Add execute permission to the file
        int chmodRet = QProcess::execute(QString(QLatin1String(
                                     "chmod +x \"%1\"")).arg(mFile.fileName()));
        if (chmodRet != 0) {
            reportErrorAndDelete(tr("Unable to add executable permissions to %1")
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

    connect(this, &QProcess::errorOccurred,
            this, &CommandProcess::handleProcessError);

    connect(this, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &QObject::deleteLater);

    if (showOutput) {
        Tiled::INFO(tr("Executing: %1").arg(mFinalCommand));

        connect(this, &QProcess::readyReadStandardError, this, &CommandProcess::consoleError);
        connect(this, &QProcess::readyReadStandardOutput, this, &CommandProcess::consoleOutput);
    }

    if (!mFinalWorkingDirectory.trimmed().isEmpty())
        setWorkingDirectory(mFinalWorkingDirectory);

    start(mFinalCommand);
}

void CommandProcess::consoleOutput()
{
    Tiled::INFO(QString::fromLocal8Bit(readAllStandardOutput()));
}

void CommandProcess::consoleError()
{
    Tiled::ERROR(QString::fromLocal8Bit(readAllStandardError()));
}

void CommandProcess::handleProcessError(QProcess::ProcessError error)
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

    reportErrorAndDelete(errorStr);
}

void CommandProcess::reportErrorAndDelete(const QString &error)
{
    const QString title = tr("Error Executing %1").arg(mName);
    const QString message = error + QLatin1String("\n\n") + mFinalCommand;

    QWidget *parent = DocumentManager::instance()->widget();
    QMessageBox::warning(parent, title, message);

    // Make sure this object gets deleted if the process failed to start
    deleteLater();
}
