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

#include "preferences.h"

#include <QKeySequence>
#include <QMenu>
#include <QMimeData>

#include "qtcompat_p.h"

using namespace Tiled;

static const char *commandMimeType = "application/x-tiled-commandptr";

/**
 * Constructs the object and parses the users settings to allow easy
 * programmatic access to the command list.
 */
CommandDataModel::CommandDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void CommandDataModel::setCommands(const QVector<Command> &commands)
{
    beginResetModel();
    mCommands = commands;
    endResetModel();
}

/**
 * Remove the given row or rows from the model.
 */
bool CommandDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row + count > mCommands.size())
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    mCommands.erase(mCommands.begin() + row, mCommands.begin() + row + count);
    endRemoveRows();

    return true;
}

/**
 * Deletes the commands associated with the given row \a indices.
 */
void CommandDataModel::removeRows(QModelIndexList indices)
{
    while (!indices.empty()) {
        const int row = indices.takeFirst().row();
        if (row >= mCommands.size())
            continue;

        beginRemoveRows(QModelIndex(), row, row);
        mCommands.removeAt(row);

        // Decrement later indices since we removed a row
        for (QModelIndex &index : indices)
            if (index.row() > row)
                index = index.sibling(index.row() - 1, index.column());

        endRemoveRows();
    }
}

/**
 * Returns the number of rows (this includes the <New Command> row).
 */
int CommandDataModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mCommands.size() + 1;
}

/**
 * Returns the number of columns.
 */
int CommandDataModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 3;
}

/**
 * Returns the data at \a index for the given \a role.
 */
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
            if (index.column() == ShortcutColumn)
                return command.shortcut;
        } else {
            if (index.column() == NameColumn) {
                if (role == Qt::EditRole)
                    return QString();
                else
                    return tr("<new command>");
            }
        }
        break;

    case Qt::ToolTipRole:
        if (isNormalRow) {
            if (index.column() == NameColumn)
                return tr("Set a name for this command");
            if (index.column() == ShortcutColumn)
                return tr("Shortcut for this command");
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

/**
 * Sets the data at \a index to the given \a value for the given \a role.
 */
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
                }
            }
            break;
        }

        case Qt::CheckStateRole:
            if (index.column() == EnabledColumn) {
                command.isEnabled = value.toInt() > 0;
                isModified = true;
            }
            break;
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

/**
 * Returns flags for the item at \a index.
 */
Qt::ItemFlags CommandDataModel::flags(const QModelIndex &index) const
{
    const bool isNormalRow = index.row() < mCommands.size();
    Qt::ItemFlags f = QAbstractTableModel::flags(index);

    if (isNormalRow) {
        f |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        if (index.column() == EnabledColumn)
            f |= Qt::ItemIsUserCheckable;
        else if (index.column() == NameColumn)
            f |= Qt::ItemIsEditable;
    } else {
        f |= Qt::ItemIsDropEnabled;
        if (index.column() == NameColumn)
            f |= Qt::ItemIsEditable;
    }

    return f;
}

/**
 * Returns the header data for the given \a section and \a role.
 * \a orientation should be Qt::Horizontal.
 */
QVariant CommandDataModel::headerData(int section, Qt::Orientation orientation,
                                                   int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    const char *sectionLabels[3] = {
        QT_TR_NOOP("Name"),
        QT_TR_NOOP("Shortcut"),
        QT_TR_NOOP("Enable") };

    return tr(sectionLabels[section]);
}

/**
 * Returns a menu containing a list of appropriate actions for the item at
 * \a index, or 0 if there are no actions for the index.
 */
QMenu *CommandDataModel::contextMenu(QWidget *parent, const QModelIndex &index)
{
    QMenu *menu = nullptr;
    const int row = index.row();

    if (row >= 0 && row < mCommands.size()) {
        menu = new QMenu(parent);

        if (row > 0)
            menu->addAction(tr("Move Up"), [=] { moveUp(row); });

        if (row + 1 < mCommands.size())
            menu->addAction(tr("Move Down"), [=] { moveUp(row + 1); });

        menu->addSeparator();
        menu->addAction(tr("Execute"), [=] { execute(row); });
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        menu->addAction(tr("Execute in Terminal"), [=] { executeInTerminal(row); });
#endif

        menu->addSeparator();
        menu->addAction(tr("Delete"), [=] { removeRow(row); });
    }

    return menu;
}

/**
 * Returns mime data for the first index in \a indexes.
 */
QMimeData *CommandDataModel::mimeData(const QModelIndexList &indices) const
{
    int row = -1;

    for (const QModelIndex &index : indices) {
        // Only generate mime data on command rows
        if (index.row() < 0 || index.row() >= mCommands.size())
            return nullptr;

        // Currently only one row at a time is supported for drags
        // Note: we can get multiple indexes in the same row (different columns)
        if (row != -1 && index.row() != row)
            return nullptr;

        row = index.row();
    }

    const Command &command = mCommands[row];
    QMimeData* mimeData = new QMimeData();

    // Text data is used if command is dragged to a text editor or terminal
    mimeData->setText(command.finalCommand());

    // Ptr is used if command is dragged onto another command
    // We could store the index instead, the only difference would be that if
    // the item is moved or deleted somehow during the drag, the ptr approach
    // will result in a no-op instead of moving the wrong thing.
    const Command *addr = &command;
    mimeData->setData(QLatin1String(commandMimeType),
                      QByteArray((const char *)&addr, sizeof(addr)));

    return mimeData;
}

/**
 * Returns a list of mime types that can represent a command.
 */
QStringList CommandDataModel::mimeTypes() const
{
    return {
        QLatin1String("text/plain"),
        QLatin1String(commandMimeType)
    };
}

/**
 * Returns the drop actions that can be performed.
 */
Qt::DropActions CommandDataModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

/**
 * Handles dropping of mime data onto <i>parent</i>.
 */
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
                // move the src command into the position of the dst command.
                if (dstRow < mCommands.size())
                    return move(srcRow, dstRow);

                // If a command is dropped elsewhere, create a copy of it
                if (dstRow == mCommands.size()) {
                    auto copy = *addr;
                    copy.name = tr("%1 (copy)").arg(addr->name);
                    copy.shortcut = QKeySequence();
                    append(copy);
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
            Command newCommand;
            newCommand.name = tr("New command");
            newCommand.executable = data->text();
            append(newCommand);
            return true;
        }
    }

    return false;
}

void CommandDataModel::setExecutable(const QModelIndex &index, const QString &value)
{
    const bool isNormalRow = index.row() < mCommands.size();

    if (isNormalRow)
        mCommands[index.row()].executable = value;
}

void CommandDataModel::setArguments(const QModelIndex &index, const QString &value)
{
    const bool isNormalRow = index.row() < mCommands.size();

    if (isNormalRow)
        mCommands[index.row()].arguments = value;
}

void CommandDataModel::setWorkingDirectory(const QModelIndex &index, const QString &value)
{
    const bool isNormalRow = index.row() < mCommands.size();

    if (isNormalRow)
        mCommands[index.row()].workingDirectory = value;
}

void CommandDataModel::setShortcut(const QModelIndex &index, const QKeySequence &value)
{
    const bool isNormalRow = index.row() < mCommands.size();

    if (isNormalRow) {
        mCommands[index.row()].shortcut = value;

        QModelIndex shortcutIndex = this->index(index.row(), ShortcutColumn);
        emit dataChanged(shortcutIndex, shortcutIndex);
    }
}

void CommandDataModel::setShowOutput(const QModelIndex &index, bool value)
{
    const bool isNormalRow = index.row() < mCommands.size();

    if (isNormalRow)
        mCommands[index.row()].showOutput = value;
}

void CommandDataModel::setSaveBeforeExecute(const QModelIndex &index, bool value)
{
    const bool isNormalRow = index.row() < mCommands.size();

    if (isNormalRow)
        mCommands[index.row()].saveBeforeExecute = value;
}

Command CommandDataModel::command(const QModelIndex &index) const
{
    const bool isNormalRow = index.row() < mCommands.size();

    if (isNormalRow)
        return mCommands[index.row()];
    else
        return Command();
}

/**
 * Moves the command at \a commandIndex to \a newIndex.
 */
bool CommandDataModel::move(int commandIndex, int newIndex)
{
    if (commandIndex < 0 || commandIndex >= mCommands.size() ||
        newIndex < 0 || newIndex >= mCommands.size() ||
        newIndex == commandIndex)
        return false;

    if (!beginMoveRows(QModelIndex(), commandIndex, commandIndex, QModelIndex(),
                       newIndex > commandIndex ? newIndex + 1 : newIndex))
        return false;

    if (commandIndex - newIndex == 1 || newIndex - commandIndex == 1) {
        // Swapping is probably more efficient than removing/inserting
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        qSwap(mCommands[commandIndex], mCommands[newIndex]);
#else
        mCommands.swapItemsAt(commandIndex, newIndex);
#endif
    } else {
        const Command command = mCommands.at(commandIndex);
        mCommands.removeAt(commandIndex);
        mCommands.insert(newIndex, command);
    }

    endMoveRows();

    return true;
}

/**
 * Appends \a command to the command list.
 */
void CommandDataModel::append(const Command &command)
{
    beginInsertRows(QModelIndex(), mCommands.size(), mCommands.size());
    mCommands.append(command);
    endInsertRows();
}

/**
 * Moves the command at \a commandIndex up one index, if possible.
 */
void CommandDataModel::moveUp(int commandIndex)
{
    move(commandIndex, commandIndex - 1);
}

/**
 * Executes the command at \a commandIndex.
 */
void CommandDataModel::execute(int commandIndex) const
{
    mCommands.at(commandIndex).execute();
}

/**
 * Executes the command at \a commandIndex within the systems native
 * terminal if available.
 */
void CommandDataModel::executeInTerminal(int commandIndex) const
{
    mCommands.at(commandIndex).execute(true);
}
