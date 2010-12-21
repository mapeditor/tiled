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

#ifndef COMMANDDATAMODEL_H
#define COMMANDDATAMODEL_H

#include <QAbstractTableModel>
#include <QSettings>

namespace Tiled {
namespace Internal {

class DocumentManager;

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
};

class CommandDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum { NameColumn, CommandColumn, EnabledColumn };

    /**
      * Constructs the object and parses the users settings to allow easy
      * programmatic access to the command list.
      */
    CommandDataModel();

    /**
      * Saves the data to the users preferences.
      */
    void commit();

    /**
      * Returns whether saving before executing commands is enabled.
      */
    bool saveBeforeExecute() const { return mSaveBeforeExecute; }

    /**
      * Enables or disables saving before executing commands.
      */
    void setSaveBeforeExecute(bool enabled) { mSaveBeforeExecute = enabled; }

    /**
      * Returns the first enabled command in the list, or an empty
      * disabled command if there are no enabled commands.
      */
    Command firstEnabledCommand() const;

    /**
      * Returns a list of all the commands.
      */
    const QList<Command> &allCommands() const { return mCommands; }

    /**
      * Deletes the commands associated with the given row <i>indices</i>.
      */
    void deleteCommands(QModelIndexList indices);

    /**
     * Returns the number of rows (this includes the <New Command> row).
     */
    int rowCount(const QModelIndex &) const;

    /**
     * Returns the number of columns.
     */
    int columnCount(const QModelIndex &) const;

    /**
     * Returns the data at <i>index</i> for the given <i>role</i>.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /**
     * Sets the data at <i>index</i> to the given <i>value</i>.
     * for the given <i>role</i>
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    /**
     * Returns flags for the item at <i>index</i>.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
     * Returns the header data for the given <i>section</i> and <i>role</i>.
     * <i>orientation</i> should be Qt::Horizontal.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::EditRole) const;

private:

    QSettings mSettings;
    QList<Command> mCommands;
    bool mSaveBeforeExecute;
};

} // namespace Internal
} // namespace Tiled

#endif // COMMANDDATAMODEL_H
