/*
 * commanddatamodel.cpp
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

#include "commanddatamodel.h"

using namespace Tiled;
using namespace Tiled::Internal;

CommandDataModel::CommandDataModel()
{
    // Load saveBeforeExecute option

    QVariant s = mSettings.value(QLatin1String("saveBeforeExecute"), true);
    mSaveBeforeExecute = s.toBool();

    // Load command list

    const QVariant variant = mSettings.value(QLatin1String("commandList"));
    const QList<QVariant> commands = variant.toList();

    const QString namePref = QLatin1String("Name");
    const QString commandPref = QLatin1String("Command");
    const QString enablePref = QLatin1String("Enabled");
    foreach (const QVariant &commandVariant, commands) {
        QHash<QString, QVariant> commandHash = commandVariant.toHash();

        Command command;
        if (commandHash.contains(enablePref))
            command.isEnabled = commandHash[enablePref].toBool();
        if (commandHash.contains(namePref))
            command.name = commandHash[namePref].toString();
        if (commandHash.contains(commandPref))
            command.command = commandHash[commandPref].toString();

        mCommands.append(command);
    }

    // Add default commands the first time the app has booted up.
    // This is useful on it's own and helps demonstrate how to use the commands.

    const QString addPrefStr = QLatin1String("addedDefaultCommands");
    const bool addedCommands = mSettings.value(addPrefStr, false).toBool();
    if (!addedCommands) {

        QString defaultTextEditor;
#ifdef Q_WS_X11
        defaultTextEditor = QLatin1String("gedit");
#elif defined(Q_WS_MAC)
        defaultTextEditor = QLatin1String("/Applications/TextEdit.app/Contents/"
                                          "MacOS/TextEdit");
#endif

        if (!defaultTextEditor.isEmpty()) {
            // Disable default commands by default so user gets an informative
            // warning when clicking the command button for the first time
            Command command(false);
            command.name = tr("Open in text editor");
            command.command = defaultTextEditor + QLatin1String(" %mapfile");
            mCommands.push_back(command);
        }

        commit();
        mSettings.setValue(addPrefStr, true);
    }
}

void CommandDataModel::commit()
{
    // Save saveBeforeExecute option

    mSettings.setValue(QLatin1String("saveBeforeExecute"), mSaveBeforeExecute);

    // Save command list

    QList<QVariant> commands;

    foreach (const Command &command, mCommands) {
        QHash<QString, QVariant> commandHash;
        commandHash[QLatin1String("Enabled")] = command.isEnabled;
        commandHash[QLatin1String("Name")] = command.name;
        commandHash[QLatin1String("Command")] = command.command;
        commands.append(commandHash);
    }

    mSettings.setValue(QLatin1String("commandList"), commands);
}

Command CommandDataModel::firstEnabledCommand() const
{
    foreach (const Command &command, mCommands)
        if (command.isEnabled)
            return command;

    return Command(false);
}

void CommandDataModel::deleteCommands(QModelIndexList indices)
{
    while (!indices.empty()) {
        const int row = indices.takeFirst().row();
        if (row >= mCommands.size())
            continue;

        beginRemoveRows(QModelIndex(), row, row);
        mCommands.removeAt(row);

        // Decrement later indicies since we removed a row
        for (QModelIndexList::iterator i = indices.begin(); i != indices.end();
                                                                            ++i)
            if (i->row() > row)
                *i = i->sibling(i->row() - 1, i->column());

        endRemoveRows();
    }
}

int CommandDataModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mCommands.size() + 1;
}

int CommandDataModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 3;
}

QVariant CommandDataModel::data(const QModelIndex &index, int role) const
{
    const bool isNormalRow = index.row() < mCommands.size();
    Command command;
    if (isNormalRow)
        command = mCommands[index.row()];

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (isNormalRow) {
            if (index.column() == NameColumn)
                return command.name;
            if (index.column() == CommandColumn)
                return command.command;
        } else
            if (index.column() == NameColumn) {
                if (role == Qt::EditRole)
                    return QString();
                else
                    return tr("<new command>");
            }

        break;

    case Qt::ToolTipRole:
        if (isNormalRow) {
            if (index.column() == NameColumn)
                return tr("Set a name for this command");
            if (index.column() == CommandColumn)
                return tr("Set the shell command to execute");
            if (index.column() == EnabledColumn)
                return tr("Show or hide this command in the command list");
        } else
            if (index.column() == NameColumn)
                return tr("Add a new command");
        break;

    case Qt::CheckStateRole:
        if (isNormalRow && index.column() == EnabledColumn)
            return command.isEnabled ? 2 : 0;
        break;
    }

    return QVariant();
}

bool CommandDataModel::setData(const QModelIndex &index,
                                     const QVariant &value, int role)
{
    const bool isNormalRow = index.row() < mCommands.size();
    bool isModified = false;
    bool shouldAppend = false;
    Command command;

    if (isNormalRow) {
        // Get the command as it exists already
        command = mCommands[index.row()];

        // Modify the command based on the passed date
        switch (role) {
        case Qt::EditRole: {
            const QString text = value.toString();
            if (!text.isEmpty()) {
                if (index.column() == NameColumn) {
                    command.name = value.toString();
                    isModified = true;
                } else if (index.column() == CommandColumn) {
                    command.command = value.toString();
                    isModified = true;
                }
            }
        }

        case Qt::CheckStateRole:
            if (index.column() == EnabledColumn) {
                command.isEnabled = value.toInt() > 0;
                isModified = true;
            }
        }

    } else {

        // If final row was edited, insert the new command
        if (role == Qt::EditRole && index.column() == NameColumn) {
            command.name = value.toString();
            if (!command.name.isEmpty()
              && command.name != tr("<new command>")) {
                isModified = true;
                shouldAppend = true;
            }
        }
    }

    if (isModified) {
        // Write the modified command to our cache
        if (shouldAppend)
            mCommands.push_back(command);
        else
            mCommands[index.row()] = command;

        // Reset if there could be new rows or reordering, else emit dataChanged
        if (shouldAppend || index.column() == NameColumn)
            reset();
        else
            emit dataChanged(index, index);
    }

    return isModified;
}

Qt::ItemFlags CommandDataModel::flags(const QModelIndex &index) const
{
    const bool isNormalRow = index.row() < mCommands.size();
    Qt::ItemFlags f = QAbstractTableModel::flags(index);

    if (isNormalRow) {
        f |= Qt::ItemIsEditable;
        if (index.column() == EnabledColumn)
            f |= Qt::ItemIsUserCheckable;
    } else {
        if (index.column() == NameColumn)
            f |= Qt::ItemIsEditable;
    }

    return f;
}

QVariant CommandDataModel::headerData(int section, Qt::Orientation orientation,
                                                   int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    const char *sectionLabels[3] = {
        QT_TR_NOOP("Name"),
        QT_TR_NOOP("Command"),
        QT_TR_NOOP("Enable") };

    return tr(sectionLabels[section]);
}
