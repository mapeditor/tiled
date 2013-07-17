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

#include <QMenu>
#include <QSignalMapper>
#include <QMimeData>

using namespace Tiled;
using namespace Tiled::Internal;

const char *commandMimeType = "application/x-tiled-commandptr";

CommandDataModel::CommandDataModel()
{
    // Load saveBeforeExecute option
    QVariant s = mSettings.value(QLatin1String("saveBeforeExecute"), true);
    mSaveBeforeExecute = s.toBool();

    // Load command list
    const QVariant variant = mSettings.value(QLatin1String("commandList"));
    const QList<QVariant> commands = variant.toList();
    foreach (const QVariant &commandVariant, commands)
        mCommands.append(Command::fromQVariant(commandVariant));

    // Add default commands the first time the app has booted up.
    // This is useful on it's own and helps demonstrate how to use the commands.

    const QString addPrefStr = QLatin1String("addedDefaultCommands");
    const bool addedCommands = mSettings.value(addPrefStr, false).toBool();
    if (!addedCommands) {

        // Disable default commands by default so user gets an informative
        // warning when clicking the command button for the first time
        Command command(false);
#ifdef Q_WS_X11
        command.command = QLatin1String("gedit %mapfile");
#elif defined(Q_OS_MAC)
        command.command = QLatin1String("open -t %mapfile");
#endif
        if (!command.command.isEmpty()) {
            command.name = tr("Open in text editor");
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
    foreach (const Command &command, mCommands)
        commands.append(command.toQVariant());
    mSettings.setValue(QLatin1String("commandList"), commands);
}

Command CommandDataModel::firstEnabledCommand() const
{
    foreach (const Command &command, mCommands)
        if (command.isEnabled)
            return command;

    return Command(false);
}

bool CommandDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row + count > mCommands.size())
        return false;

    beginRemoveRows(parent, row, row + count);
    mCommands.erase(mCommands.begin() + row, mCommands.begin() + row + count);
    endRemoveRows();

    return true;
}

void CommandDataModel::removeRows(QModelIndexList indices)
{
    while (!indices.empty()) {
        const int row = indices.takeFirst().row();
        if (row >= mCommands.size())
            continue;

        beginRemoveRows(QModelIndex(), row, row);
        mCommands.removeAt(row);

        // Decrement later indices since we removed a row
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
        if (shouldAppend || index.column() == NameColumn) {
            beginResetModel();
            endResetModel();
        } else {
            emit dataChanged(index, index);
        }
    }

    return isModified;
}

Qt::ItemFlags CommandDataModel::flags(const QModelIndex &index) const
{
    const bool isNormalRow = index.row() < mCommands.size();
    Qt::ItemFlags f = QAbstractTableModel::flags(index);

    if (isNormalRow) {
        f |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        if (index.column() == EnabledColumn)
            f |= Qt::ItemIsUserCheckable;
    } else {
        f |= Qt::ItemIsDropEnabled;
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

QMenu *CommandDataModel::contextMenu(QWidget *parent, const QModelIndex &index)
{
    QMenu *menu = NULL;
    const int row = index.row();

    if (row >= 0 && row < mCommands.size()) {
        menu = new QMenu(parent);

        if (row > 0) {
            QAction *action = menu->addAction(tr("Move Up"));
            QSignalMapper *mapper = new QSignalMapper(action);
            mapper->setMapping(action, row);
            connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
            connect(mapper, SIGNAL(mapped(int)), SLOT(moveUp(int)));
        }

        if (row+1 < mCommands.size()) {
            QAction *action = menu->addAction(tr("Move Down"));
            QSignalMapper *mapper = new QSignalMapper(action);
            mapper->setMapping(action, row + 1);
            connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
            connect(mapper, SIGNAL(mapped(int)), SLOT(moveUp(int)));
        }

        menu->addSeparator();

        {
            QAction *action = menu->addAction(tr("Execute"));
            QSignalMapper *mapper = new QSignalMapper(action);
            mapper->setMapping(action, row);
            connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
            connect(mapper, SIGNAL(mapped(int)), SLOT(execute(int)));
        }

#if defined(Q_WS_X11) || defined(Q_OS_MAC)
        {
            QAction *action = menu->addAction(tr("Execute in Terminal"));
            QSignalMapper *mapper = new QSignalMapper(action);
            mapper->setMapping(action, row);
            connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
            connect(mapper, SIGNAL(mapped(int)), SLOT(executeInTerminal(int)));
        }
#endif

        menu->addSeparator();

        {
            QAction *action = menu->addAction(tr("Delete"));
            QSignalMapper *mapper = new QSignalMapper(action);
            mapper->setMapping(action, row);
            connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
            connect(mapper, SIGNAL(mapped(int)), SLOT(remove(int)));
        }
    }

    return menu;
}

QMimeData *CommandDataModel::mimeData(const QModelIndexList &indices) const
{
    int row = -1;

    foreach (const QModelIndex &index, indices) {
        // Only generate mime data on command rows
        if (index.row() < 0 || index.row() >= mCommands.size())
            return 0;

        // Currently only one row at a time is supported for drags
        // Note: we can get multiple indexes in the same row (different columns)
        if (row != -1 && index.row() != row)
            return 0;

        row = index.row();
    }

    const Command &command = mCommands[row];
    QMimeData* mimeData = new QMimeData();

    // Text data is used if command is dragged to a text editor or terminal
    mimeData->setText(command.finalCommand());

    // Ptr is used if command is dragged onto another command
    // We could store the index instead, the only difference would be that if
    // the item is moved or deleted shomehow during the drag, the ptr approach
    // will result in a no-op instead of moving the wrong thing.
    const Command *addr = &command;
    mimeData->setData(QLatin1String(commandMimeType),
                      QByteArray((const char *)&addr, sizeof(addr)));

    return mimeData;
}

QStringList CommandDataModel::mimeTypes() const
{
    QStringList result(QLatin1String("text/plain"));
    result.append(QLatin1String(commandMimeType));
    return result;
}

Qt::DropActions CommandDataModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

bool CommandDataModel::dropMimeData(const QMimeData *data, Qt::DropAction, int,
                                    int, const QModelIndex &parent)
{
    if (!parent.isValid())
        return false;

    const int dstRow = parent.row();

    if (data->hasFormat(QLatin1String(commandMimeType))) {

        // Get the ptr to the command that was being dragged
        const QByteArray byteData = data->data(QLatin1String(commandMimeType));
        Q_ASSERT(byteData.length() == sizeof(Command*));
        const Command *addr = *(Command**)byteData.data();

        // Find the command in the command list so we can move/copy it
        for (int srcRow = 0; srcRow < mCommands.size(); ++srcRow)
            if (addr == &mCommands[srcRow]) {

                // If a command is dropped on another command,
                // move the src command into the positon of the dst command.
                if (dstRow < mCommands.size())
                    return move(srcRow, dstRow);

                // If a command is dropped elsewhere, create a copy of it
                if (dstRow == mCommands.size()) {
                    append(Command(addr->isEnabled,
                                   tr("%1 (copy)").arg(addr->name),
                                   addr->command));
                    return true;
                }
            }
    }

    if (data->hasText()) {

        // If text is dropped on a valid command, just replace the data
        if (dstRow < mCommands.size())
            return setData(parent, data->text(), Qt::EditRole);

        // If text is dropped elsewhere, create a new command
        // Assume the dropped text is the command, not the name
        if (dstRow == mCommands.size()) {
            append(Command(true, tr("New command"), data->text()));
            return true;
        }
    }

    return false;
}

bool CommandDataModel::move(int commandIndex, int newIndex)
{
    if (commandIndex < 0 || commandIndex >= mCommands.size() ||
        newIndex < 0 || newIndex >= mCommands.size() ||
        newIndex == commandIndex)
        return false;

    if (!beginMoveRows(QModelIndex(), commandIndex, commandIndex, QModelIndex(),
                       newIndex > commandIndex ? newIndex + 1 : newIndex))
        return false;

    if (commandIndex - newIndex == 1 || newIndex - commandIndex == 1)
        // Swapping is probably more efficient than removing/inserting
        mCommands.swap(commandIndex, newIndex);
    else {
        const Command command = mCommands.at(commandIndex);
        mCommands.removeAt(commandIndex);
        mCommands.insert(newIndex, command);
    }

    endMoveRows();

    return true;
}

void CommandDataModel::append(const Command &command)
{
    beginInsertRows(QModelIndex(), mCommands.size(), mCommands.size());

    mCommands.append(command);

    endInsertRows();
}

void CommandDataModel::moveUp(int commandIndex)
{
    move(commandIndex, commandIndex - 1);
}

void CommandDataModel::execute(int commandIndex) const
{
    mCommands.at(commandIndex).execute();
}

void CommandDataModel::executeInTerminal(int commandIndex) const
{
    mCommands.at(commandIndex).execute(true);
}

void CommandDataModel::remove(int commandIndex)
{
    removeRow(commandIndex);
}
