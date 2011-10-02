/*
 * command.h
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

#ifndef COMMAND_H
#define COMMAND_H

#include <QString>
#include <QProcess>
#include <QVariant>

#ifdef Q_WS_MAC
#include <QTemporaryFile>
#endif

namespace Tiled {
namespace Internal {

struct Command
{
    Command(bool isEnabled = true,
            const QString &name = QString(),
            const QString &command = QString())
        : isEnabled(isEnabled)
        , name(name)
        , command(command) {}

    bool isEnabled;
    QString name;
    QString command;

    /**
     * Returns the final command with replaced tokens.
     */
    QString finalCommand() const;

    /**
     * Executes the command in the operating system shell or terminal
     * application.
     */
    void execute(bool inTerminal = false) const;

    /**
     * Stores this command in a QVariant.
     */
    QVariant toQVariant() const;

    /**
     * Generates a command from a QVariant.
     */
    static Command fromQVariant(const QVariant &variant);
};

class CommandProcess : public QProcess
{
    Q_OBJECT

public:
    CommandProcess(const Command &command, bool inTerminal = false);

private slots:
    void handleError(QProcess::ProcessError);

private:
    void handleError(const QString &);

    QString mName;
    QString mFinalCommand;

#ifdef Q_WS_MAC
    QTemporaryFile mFile;
#endif
};

} // namespace Internal
} // namespace Tiled

#endif // COMMAND_H
