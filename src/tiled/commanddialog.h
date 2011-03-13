/*
 * commanddialog.h
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

#ifndef COMMANDDIALOG_H
#define COMMANDDIALOG_H

#include <QDialog>
#include <QTreeView>

namespace Ui {
class CommandDialog;
}

namespace Tiled {
namespace Internal {

class CommandDataModel;

class CommandDialog : public QDialog
{
    Q_OBJECT

public:
    CommandDialog(QWidget *parent = 0);
    ~CommandDialog();

    /**
      * Saves the changes to the users preferences.
      * Automatically called when the dialog is accepted.
      */
    void accept();

private:
    Ui::CommandDialog *mUi;
};

class CommandTreeView : public QTreeView
{
    Q_OBJECT

public:
    CommandTreeView(QWidget *parent);
    ~CommandTreeView();

    /**
      * Returns the model used by this view in CommandDataMode form.
      */
    CommandDataModel *model() const { return mModel; }

private slots:
    /**
      * Displays a context menu for the item at <i>event</i>'s position.
      */
    void contextMenuEvent(QContextMenuEvent *event);

    /**
      * Fixes the selection after rows have been removed.
      */
    void handleRowsRemoved(const QModelIndex &parent, int start, int end);

    /**
      * Gets the currently selected rows and tells the model to delete them.
      */
    void removeSelectedCommands();

private:
    CommandDataModel *mModel;
};

} // namespace Internal
} // namespace Tiled

#endif // COMMANDDIALOG_H
