/*
 * commanddatamodel.h
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

#pragma once

#include "command.h"

#include <QAbstractTableModel>

class QMenu;

namespace Tiled {

class CommandDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum { NameColumn, ShortcutColumn, EnabledColumn };

    CommandDataModel(QObject *parent = nullptr);

    void setCommands(const QVector<Command> &commands);
    const QVector<Command> &commands() const;

    bool removeRows(int row, int count,
                    const QModelIndex &parent = QModelIndex()) override;

    void removeRows(QModelIndexList indices);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::EditRole) const override;

    QMenu *contextMenu(QWidget *parent, const QModelIndex &index);

    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QStringList mimeTypes() const override;

    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                      int column, const QModelIndex &parent) override;

    void setExecutable(const QModelIndex &index, const QString &value);
    void setArguments(const QModelIndex &index, const QString &value);
    void setWorkingDirectory(const QModelIndex &index, const QString &value);
    void setShortcut(const QModelIndex &index, const QKeySequence &value);
    void setShowOutput(const QModelIndex &index, bool value);
    void setSaveBeforeExecute(const QModelIndex &index, bool value);

    Command command(const QModelIndex &index) const;

public slots:
    bool move(int commandIndex, int newIndex);
    void append(const Command &command);
    void moveUp(int commandIndex);

    void execute(int commandIndex) const;
    void executeInTerminal(int commandIndex) const;

private:
    QVector<Command> mCommands;
};


/**
 * Returns a list of all the commands.
 */
inline const QVector<Command> &CommandDataModel::commands() const
{
    return mCommands;
}

} // namespace Tiled
