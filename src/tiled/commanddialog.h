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

#pragma once

#include <QDialog>
#include <QTreeView>

namespace Ui {
class CommandDialog;
}

namespace Tiled {

class CommandDataModel;

class CommandDialog : public QDialog
{
    Q_OBJECT

public:
    CommandDialog(QWidget *parent = nullptr);
    ~CommandDialog();

    /**
      * Saves the changes to the users preferences.
      * Automatically called when the dialog is closed.
      */
    void closeEvent(QCloseEvent *event) override;

public slots:
    void setShortcut(const QKeySequence &keySequence);

    void setSaveBeforeExecute(int state);

    void setShowOutput(int state);

    void setExecutable(const QString &text);

    void setArguments(const QString &text);

    void setWorkingDirectory(const QString &text);

    void updateWidgets(const QModelIndex &current, const QModelIndex&);

    void browseExecutable();

    void browseWorkingDirectory();

private:
    Ui::CommandDialog *mUi;
};

class CommandTreeView : public QTreeView
{
    Q_OBJECT

public:
    CommandTreeView(QWidget *parent);

    /**
      * Returns the model used by this view in CommandDataMode form.
      */
    CommandDataModel *model() const { return mModel; }

protected:
    /**
      * Displays a context menu for the item at <i>event</i>'s position.
      */
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    /**
      * Fixes the selection after rows have been removed.
      */
    void handleRowsRemoved(const QModelIndex &parent, int start, int end);

    /**
      * Gets the currently selected rows and tells the model to delete them.
      */
    void removeSelectedCommands();

    CommandDataModel *mModel;
};

} // namespace Tiled
