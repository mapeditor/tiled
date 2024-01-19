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

#include "actionmanager.h"
#include "commandmanager.h"
#include "documentmanager.h"
#include "logginginterface.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "projectmanager.h"
#include "world.h"
#include "worlddocument.h"  // used to know that WorldDocument is a Document
#include "worldmanager.h"

#include <QAction>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QUndoStack>

using namespace Tiled;

namespace Tiled {

class CommandProcess : public QProcess
{
    Q_OBJECT

public:
    CommandProcess(const Command &command, bool inTerminal = false, bool showOutput = true);

private:
    void consoleOutput();
    void consoleError();
    void handleProcessError(QProcess::ProcessError);

    void reportErrorAndDelete(const QString &);

    QString mName;
    QString mFinalCommand;

#ifdef Q_OS_MAC
    QTemporaryFile mFile;
#endif
};


static QString replaceVariables(const QString &string, bool quoteValues = true)
{
    QString finalString = string;
    QString replaceString = quoteValues ? QStringLiteral("\"%1\"") :
                                          QStringLiteral("%1");

    // Perform variable replacement
    if (Document *document = DocumentManager::instance()->currentDocument()) {
        const QString fileName = document->fileName();
        QFileInfo fileInfo(fileName);
        const QString mapPath = fileInfo.absolutePath();
        const QString projectPath = QFileInfo(ProjectManager::instance()->project().fileName()).absolutePath();

        finalString.replace(QLatin1String("%mapfile"), replaceString.arg(fileName));
        finalString.replace(QLatin1String("%mappath"), replaceString.arg(mapPath));
        finalString.replace(QLatin1String("%projectpath"), replaceString.arg(projectPath));

        if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
            if (const Layer *layer = mapDocument->currentLayer()) {
                finalString.replace(QLatin1String("%layername"),
                                    replaceString.arg(layer->name()));
            }
        } else if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
            QStringList selectedTileIds;
            for (const Tile *tile : tilesetDocument->selectedTiles())
                selectedTileIds.append(QString::number(tile->id()));

            finalString.replace(QLatin1String("%tileid"),
                                replaceString.arg(selectedTileIds.join(QLatin1Char(','))));
        }

        if (const MapObject *currentObject = dynamic_cast<MapObject *>(document->currentObject())) {
            // For compatility with Tiled < 1.9
            finalString.replace(QLatin1String("%objecttype"),
                                replaceString.arg(currentObject->className()));

            finalString.replace(QLatin1String("%objectclass"),
                                replaceString.arg(currentObject->className()));
            finalString.replace(QLatin1String("%objectid"),
                                replaceString.arg(currentObject->id()));
        }

        if (const World *world = WorldManager::instance().worldForMap(fileName)) {
            finalString.replace(QLatin1String("%worldfile"), replaceString.arg(world->fileName));
        }
    }

    return finalString;
}

} // namespace Tiled


QString Command::finalWorkingDirectory() const
{
    QString finalWorkingDirectory = replaceVariables(workingDirectory, false);
    QString finalExecutable = replaceVariables(executable);
    QFileInfo mFile(finalExecutable);

    if (!mFile.exists())
        mFile = QFileInfo(QStandardPaths::findExecutable(finalExecutable));

    finalWorkingDirectory.replace(QLatin1String("%executablepath"),
                                  mFile.absolutePath());

    return finalWorkingDirectory;
}

/**
 * Returns the final command with replaced tokens.
 */
QString Command::finalCommand() const
{
    QString exe = executable.trimmed();

    // Quote the executable when not already done, to make it work even when
    // the path contains spaces.
    if (!exe.startsWith(QLatin1Char('"')) && !exe.startsWith(QLatin1Char('\'')))
        exe.prepend(QLatin1Char('"')).append(QLatin1Char('"'));

    QString finalCommand = QStringLiteral("%1 %2").arg(exe, arguments);
    return replaceVariables(finalCommand);
}

/**
 * Executes the command in the operating system shell or terminal
 * application.
 */
void Command::execute(bool inTerminal) const
{
    if (saveBeforeExecute) {
        ActionManager::instance()->action("Save")->trigger();

        if (Document *document = DocumentManager::instance()->currentDocument()) {
            if (document->type() == Document::MapDocumentType) {
                if (const World *world = WorldManager::instance().worldForMap(document->fileName())) {
                    auto worldDocument = DocumentManager::instance()->ensureWorldDocument(world->fileName);
                    DocumentManager::instance()->saveDocument(worldDocument);
                }
            }
        }
    }

    // Start the process
    new CommandProcess(*this, inTerminal, showOutput);
}

/**
 * Stores this command in a QVariant.
 */
QVariantHash Command::toVariant() const
{
    return QVariantHash {
        { QStringLiteral("arguments"), arguments },
        { QStringLiteral("command"), executable },
        { QStringLiteral("enabled"), isEnabled },
        { QStringLiteral("name"), name },
        { QStringLiteral("saveBeforeExecute"), saveBeforeExecute },
        { QStringLiteral("shortcut"), shortcut },
        { QStringLiteral("showOutput"), showOutput },
        { QStringLiteral("workingDirectory"), workingDirectory },
    };
}

/**
 * Generates a command from a QVariant.
 */
Command Command::fromVariant(const QVariant &variant)
{
    const auto hash = variant.toHash();

    auto read = [&] (const QString &prop) {
        if (hash.contains(prop))
            return hash.value(prop);

        QString oldProp = prop.at(0).toUpper() + prop.mid(1);
        return hash.value(oldProp);
    };

    const QVariant arguments = read(QStringLiteral("arguments"));
    const QVariant enable = read(QStringLiteral("enabled"));
    const QVariant executable = read(QStringLiteral("command"));
    const QVariant name = read(QStringLiteral("name"));
    const QVariant saveBeforeExecute = read(QStringLiteral("saveBeforeExecute"));
    const QVariant shortcut = read(QStringLiteral("shortcut"));
    const QVariant showOutput = read(QStringLiteral("showOutput"));
    const QVariant workingDirectory = read(QStringLiteral("workingDirectory"));

    Command command;

    command.arguments = arguments.toString();
    command.isEnabled = enable.toBool();
    command.executable = executable.toString();
    command.name = name.toString();
    command.saveBeforeExecute = saveBeforeExecute.toBool();
    command.shortcut = shortcut.value<QKeySequence>();
    command.showOutput = showOutput.toBool();
    command.workingDirectory = workingDirectory.toString();

    return command;
}

CommandProcess::CommandProcess(const Command &command, bool inTerminal, bool showOutput)
    : QProcess(DocumentManager::instance())
    , mName(command.name)
    , mFinalCommand(command.finalCommand())
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
        static bool hasGnomeTerminal = QProcess::execute(QLatin1String("which"),
                                                         QStringList(QLatin1String("gnome-terminal"))) == 0;

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
        int chmodRet = QProcess::execute(QStringLiteral("chmod"),
                                         { QStringLiteral("+x"), mFile.fileName() });
        if (chmodRet != 0) {
            reportErrorAndDelete(tr("Unable to add executable permissions to %1")
                                 .arg(mFile.fileName()));
            return;
        }

        // Use open command to launch the command in the terminal
        // -W makes it not return immediately
        // -n makes it open a new instance of terminal if it is open already
        mFinalCommand = QStringLiteral("open -W -n \"%1\"")
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

    const QString finalWorkingDirectory = command.finalWorkingDirectory();
    if (!finalWorkingDirectory.trimmed().isEmpty())
        setWorkingDirectory(finalWorkingDirectory);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    start(mFinalCommand);
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QStringList args = QProcess::splitCommand(mFinalCommand);
    const QString executable = args.takeFirst();
    start(executable, args);
#else
    startCommand(mFinalCommand);
#endif
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

#include "command.moc"
